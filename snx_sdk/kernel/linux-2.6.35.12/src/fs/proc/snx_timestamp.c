#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/time.h>

#include <linux/timex.h>

static int snx_timestamp_proc_show(struct seq_file *m, void *v)
{
	snx_show_timestamp(m);

	return 0;
}

static int snx_timestamp_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, snx_timestamp_proc_show, NULL);
}

static const struct file_operations snx_timestamp_proc_fops = {
	.open		= snx_timestamp_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init proc_snx_timestamp_init(void)
{
	proc_create("snx_timestamp", 0, NULL, &snx_timestamp_proc_fops);
	return 0;
}
module_init(proc_snx_timestamp_init);
