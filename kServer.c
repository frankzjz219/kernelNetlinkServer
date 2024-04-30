#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <net/net_namespace.h>
#include <net/netlink.h>
#include <linux/skbuff.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/string.h> // Needed for strcat and strncat functions

#define DEVICE_NAME "kernel net server"
#define CLASS_NAME "server"

static int major_number;
static struct class* example_class = NULL;
static struct device* example_device = NULL;
static struct cdev example_cdev;
/***************************网络操作部分***************************/
#define NETLINK_USER 31


static void nl_recv_msg(struct sk_buff *skb);

static struct sock *nl_sk = NULL;
static struct proc_dir_entry *proc_entry;  // proc文件入口
static int message_count = 0;  // 消息计数器
static int total_bytes = 0;    // 总字节数计数器
// static int netLinkPid = 0;

/**************************************************************/
// netlink socket的回调函数

static void nl_recv_msg(struct sk_buff *skb) {
    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    char reply_msg[256] = {0};
    char *user_msg; // Pointer to store user message
    int msg_size;
    int res;
    if(!skb)
    {
        printk(KERN_ERR "kServer: NULL pointer of skb!\n");
    }

    // printk(KERN_INFO "kServer: entering %s\n", __FUNCTION__);
    if (skb->len < nlmsg_total_size(0)) 
    {
        printk(KERN_ERR "kServer: received invalid skb length\n");
        return;
    }
    nlh = (struct nlmsghdr *)skb->data;
    // Calculate the message size
    msg_size = NLMSG_PAYLOAD(nlh, 0);
    if (msg_size < 0) {
        printk(KERN_ERR "kServer: received invalid payload size\n");
        return;
    }
    
    pid = nlh->nlmsg_pid;
    user_msg = NLMSG_DATA(nlh);
    if (msg_size > 0) {
        user_msg[msg_size] = '\0'; // Null-terminate the message
    }
    if(msg_size == 0)
    {
        printk(KERN_ERR "kServer: got empty message!\n");
        return;
    }
    else printk(KERN_INFO "kServer: received message: [%s], size: %d\n", user_msg, msg_size);

    // Construct the reply message by appending the user message to the predefined response
    snprintf(reply_msg, sizeof(reply_msg), "kServer: %s", user_msg);
    size_t reply_len = strlen(reply_msg)+1;
    // 更新统计数据
    message_count++;
    total_bytes += msg_size;

    skb_out = nlmsg_new(reply_len, 0);
    if (!skb_out) {
        printk(KERN_ERR "kServer: Failed to allocate new skb\n");
        return;
    } 

    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, reply_len, 0);
    if (!nlh) {
        printk(KERN_ERR "kServer: Failed to put nlmsg\n");
        nlmsg_free(skb_out);
        return;
    }
    // Copy the constructed message into the Netlink message payload
    
    size_t payload_size = NLMSG_PAYLOAD(nlh, 0);
    // printk(KERN_INFO "kernel payload size: %ld\n", payload_size);
    // printk(KERN_INFO "kServer: 下发信息长度%ld\n", reply_len);
    strncpy((char*)nlmsg_data(nlh), (char*)reply_msg, reply_len+1);
    ((char*)nlmsg_data(nlh))[payload_size] = '\0'; // 添加结束符
    char* dataToSend = NLMSG_DATA(nlh);
    printk(KERN_INFO "下发信息为: [%s]\n", dataToSend);
    res = nlmsg_unicast(nl_sk, skb_out, pid);

    if (res < 0)
        printk(KERN_ERR "kServer: Error while sending back to user\n");
}


// proc文件的读取操作
static ssize_t proc_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos) {
    char buffer[128];
    int len;

    len = sprintf(buffer, "Messages received: %d\nTotal bytes: %d\n", message_count, total_bytes);
    return simple_read_from_buffer(user_buf, count, ppos, buffer, len);
}
// proc文件操作
static const struct proc_ops proc_file_fops = {
    .proc_read = proc_read,
};

// 文件操作结构体
static struct file_operations fops ={};

static int __init init_server(void) {
    printk(KERN_INFO "kServer: Initializing the simple kernel server\n");

    // 动态分配主设备号
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "simple failed to register a major number\n");
        return major_number;
    }

    // 注册设备类
    example_class = class_create(CLASS_NAME);
    if (IS_ERR(example_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(example_class);
    }

    printk(KERN_INFO "kServer: device class created correctly\n");

    // 注册设备驱动
    example_device = device_create(example_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(example_device)) {
        class_destroy(example_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(example_device);
    }

    cdev_init(&example_cdev, &fops);
    if (cdev_add(&example_cdev, MKDEV(major_number, 0), 1) < 0) {
        device_destroy(example_class, MKDEV(major_number, 0));
        class_destroy(example_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "Failed to add cdev\n");
        return -1;
    }

    // 创建netlink socket
    struct netlink_kernel_cfg cfg = {
        .input = nl_recv_msg,
    };
    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sk) {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }
    // netLinkPid = nl_sk->sk_socket->sk_netlink_pid;

    printk(KERN_INFO "kServer: netlink created successfully\n");
    // 创建proc文件
    proc_entry = proc_create("netlink_stats", 0666, NULL, &proc_file_fops);
    if (!proc_entry) {
        printk(KERN_ALERT "Error creating proc entry.\n");
        netlink_kernel_release(nl_sk);
        return -ENOMEM;
    }
    printk(KERN_INFO "kServer: proc created successfully\n");
    return 0;
}

static void __exit exit_server(void) {
    cdev_del(&example_cdev);
    device_destroy(example_class, MKDEV(major_number, 0));
    class_destroy(example_class);
    unregister_chrdev(major_number, DEVICE_NAME);

    // 释放netlink socket
    netlink_kernel_release(nl_sk);
    // 删除proc文件
    proc_remove(proc_entry);  
    printk(KERN_INFO "kServer: Goodbye from the kernel server!\n");
}


module_init(init_server);
module_exit(exit_server);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Frank·Zhang");
MODULE_DESCRIPTION("A simple Linux kernel server");