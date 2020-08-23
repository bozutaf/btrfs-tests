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
 * sudo losetup -d /dev/loop0
 */

void print_feature_flags(struct btrfs_ioctl_feature_flags flags) {

    printf("***compat flags: %llu\n", flags.compat_flags);

    printf("***compat ro flags:");

    if(flags.compat_ro_flags == 0) {
        printf(" 0\n");
    }

    else {

        printf("\n");

        if(flags.compat_ro_flags &&
           BTRFS_FEATURE_COMPAT_RO_FREE_SPACE_TREE) {
            printf("BTRFS_FEATURE_COMPAT_RO_FREE_SPACE_TREE\n");
        }

        if(flags.compat_ro_flags &&
           BTRFS_FEATURE_COMPAT_RO_FREE_SPACE_TREE_VALID) {
            printf("BTRFS_FEATURE_COMPAT_RO_FREE_SPACE_TREE_VALID\n");
        }
    }

    printf("***incompat flags:");

    if(flags.incompat_flags == 0) {
        printf(" 0\n");
    }

    else {

        printf("\n");

        if(flags.incompat_flags &
           BTRFS_FEATURE_INCOMPAT_MIXED_BACKREF) {
            printf("BTRFS_FEATURE_INCOMPAT_MIXED_BACKREF\n");
        }

        if(flags.incompat_flags &
           BTRFS_FEATURE_INCOMPAT_DEFAULT_SUBVOL) {
            printf("BTRFS_FEATURE_INCOMPAT_DEFAULT_SUBVOL\n");
        }

        if(flags.incompat_flags &
           BTRFS_FEATURE_INCOMPAT_MIXED_GROUPS) {
            printf("BTRFS_FEATURE_INCOMPAT_MIXED_GROUPS\n");
        }

        if(flags.incompat_flags &
            BTRFS_FEATURE_INCOMPAT_BIG_METADATA) {
            printf("BTRFS_FEATURE_INCOMPAT_BIG_METADATA\n");
        }

        if(flags.incompat_flags &
           BTRFS_FEATURE_INCOMPAT_COMPRESS_LZO) {
            printf("BTRFS_FEATURE_INCOMPAT_COMPRESS_LZO\n");
        }

        if(flags.incompat_flags &
           BTRFS_FEATURE_INCOMPAT_COMPRESS_ZSTD) {
            printf("BTRFS_FEATURE_INCOMPAT_COMPRESS_ZSTD\n");
        }

        if(flags.incompat_flags &
           BTRFS_FEATURE_INCOMPAT_RAID56) {
            printf("BTRFS_FEATURE_INCOMPAT_RAID56\n");
        }

        if(flags.incompat_flags &
           BTRFS_FEATURE_INCOMPAT_EXTENDED_IREF) {
            printf("BTRFS_FEATURE_INCOMPAT_EXTENDED_IREF\n");
        }

        if(flags.incompat_flags &
           BTRFS_FEATURE_INCOMPAT_SKINNY_METADATA) {
            printf("BTRFS_FEATURE_INCOMPAT_SKINNY_METADATA\n");
        }

        if(flags.incompat_flags &
           BTRFS_FEATURE_INCOMPAT_NO_HOLES) {
            printf("BTRFS_FEATURE_INCOMPAT_NO_HOLES\n");
        }

        if(flags.incompat_flags &
           BTRFS_FEATURE_INCOMPAT_METADATA_UUID) {
            printf("BTRFS_FEATURE_INCOMPAT_METADATA_UUID\n");
        }

    }

    printf("\n");
}

int main(int argc, char **argv)
{
    int volume_fd;
    struct btrfs_ioctl_feature_flags supp_flags[3] = {{0},{0},{0}};
    struct btrfs_ioctl_feature_flags set_flags[2] = {{0},{0}};
    struct btrfs_ioctl_feature_flags get_flags = {0};

    volume_fd = openat(AT_FDCWD, "/mnt/", O_RDONLY|O_NONBLOCK|O_CLOEXEC|O_DIRECTORY);

    if (ioctl(volume_fd, BTRFS_IOC_GET_SUPPORTED_FEATURES, &supp_flags) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_GET_SUPPORTED_FEATURES:\n\n");

    printf("supported flags:\n");
    print_feature_flags(supp_flags[0]);

    printf("safe to set flags:\n");
    print_feature_flags(supp_flags[1]);

    printf("safe to clear flags:\n");
    print_feature_flags(supp_flags[2]);

    printf("\n");

    set_flags[1].incompat_flags = BTRFS_FEATURE_INCOMPAT_EXTENDED_IREF;

    if (ioctl(volume_fd, BTRFS_IOC_SET_FEATURES, &set_flags) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_SET_FLAGS:\n\n");
    printf("incompat flag BTRFS_FEATURE_INCOMPAT_EXTENDED_IREF set\n\n\n");

    if (ioctl(volume_fd, BTRFS_IOC_GET_FEATURES, &get_flags) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_GET_FLAGS:\n\n");
    printf("Feature flags for mounted /mnt btrfs filesystem:\n");
    print_feature_flags(get_flags);

    return 0;
}
