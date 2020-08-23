#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/btrfs.h>
#include <string.h>

/*
 * Run the following commands before executing the program
 * to setup a test loop device with btrfs filesystem:
 *
 * qemu-img create -f raw test-disk.img 1G
 * sudo losetup -f test-disk.img
 * sudo mkfs -t btrfs /dev/loopX
 * sudo mount /dev/loop0 /mnt
 *
 * After finishing with the loop device, run following commands
 * for cleanup:
 *
 * sudo umount /mnt
 * sudo losetup -d /dev/loopX
 */

int main(int argc, char **argv)
{

    int volume_fd;
    int subvolume_fd;
    struct btrfs_ioctl_quota_ctl_args ctl = {0};
    struct btrfs_ioctl_qgroup_create_args create_args = {0};
    struct btrfs_ioctl_qgroup_assign_args assign_args = {0};
    struct btrfs_ioctl_qgroup_limit_args limit_args = {0};
    struct btrfs_qgroup_limit limit = {0};
    struct btrfs_ioctl_quota_rescan_args rescan_args = {0};

    volume_fd = openat(AT_FDCWD, "/mnt", O_RDONLY|O_NONBLOCK
                       |O_CLOEXEC|O_DIRECTORY);

    if (volume_fd < 0) {
        perror("openat");
        return 1;
    }

    ctl.cmd = BTRFS_QUOTA_CTL_ENABLE;

    if (ioctl(volume_fd, BTRFS_IOC_QUOTA_CTL, &ctl) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_QUOTA_CTL:\n");
    printf("Quota support enabled for /mnt\n\n");

    create_args.create = 1;
    create_args.qgroupid = 1;

    if (ioctl(volume_fd, BTRFS_IOC_QGROUP_CREATE, &create_args) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_QGROUP_CREATE:\n");
    printf("Quota group with id 1 created for /mnt\n\n");

    create_args.qgroupid = 2;

    if (ioctl(volume_fd, BTRFS_IOC_QGROUP_CREATE, &create_args) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_QGROUP_CREATE:\n");
    printf("Quota group with id 2 created for /mnt\n\n");

    assign_args.assign = 1;
    assign_args.src = 1;
    assign_args.dst = 2;

    if (ioctl(volume_fd, BTRFS_IOC_QGROUP_ASSIGN, &assign_args) < 0) {
        perror("ioctl BTRFS_IOC_QGROUP_ASSIGN");
        printf("\n");
    }

    limit.flags = BTRFS_QGROUP_LIMIT_MAX_RFER;
    limit.max_rfer = 100;
    limit_args.qgroupid = 1;
    limit_args.lim = limit;

    if (ioctl(volume_fd, BTRFS_IOC_QGROUP_LIMIT, &limit_args) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_QGROUP_LIMIT:\n");
    printf("Size of qgroup 1 limited to 100\n\n");

    if (ioctl(volume_fd, BTRFS_IOC_QUOTA_RESCAN, &rescan_args) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_QUOTA_RESCAN:\n");
    printf("Quota rescan started for /mnt\n\n");

    if (ioctl(volume_fd, BTRFS_IOC_QUOTA_RESCAN_WAIT) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_QUOTA_RESCAN_WAIT:\n");
    printf("Waiting for quota rescan for /mnt to finish...\n\n");

    if (ioctl(volume_fd, BTRFS_IOC_QUOTA_RESCAN_STATUS, &rescan_args) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_QUOTA_RESCAN_STATUS:\n");
    if (rescan_args.progress == 0) {
        printf("Quota rescan for /mnt finished\n\n");
    } else {
        printf("Quota rescan for /mnt still pending...");
    }

    ctl.cmd = BTRFS_QUOTA_CTL_ENABLE;

    if (ioctl(volume_fd, BTRFS_IOC_QUOTA_CTL, &ctl) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_QUOTA_CTL:\n");
    printf("Subvolume quota support enabled for /mnt\n\n");

    create_args.create = 0;
    create_args.qgroupid = 1;

    if (ioctl(volume_fd, BTRFS_IOC_QGROUP_CREATE, &create_args) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_QGROUP_CREATE:\n");
    printf("Quota group with id 1 removed for /mnt\n\n");

    create_args.qgroupid = 2;

    if (ioctl(volume_fd, BTRFS_IOC_QGROUP_CREATE, &create_args) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_QGROUP_CREATE:\n");
    printf("Quota group with id 2 removed for /mnt\n\n");

    ctl.cmd = BTRFS_QUOTA_CTL_DISABLE;

    if (ioctl(volume_fd, BTRFS_IOC_QUOTA_CTL, &ctl) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_QUOTA_CTL:\n");
    printf("Quota support disabled for /mnt\n\n");

    return 0;
}