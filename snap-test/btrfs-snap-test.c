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

/*
 * Before executing the program, a btrfs subvolume needs to be
 * created. This can be done either using the following command:
 *
 * btrfs subvolume create /mnt/test-volume
 *
 * or by executing the btrfs-subvol-test program:
 *
 * ./btrfs-subvol-test  test-volume
 */

/*
 * When executing the program, the name of the subvolume
 * and the name of the snapshot need to be mentioned:
 *
 * Example execution of the program:
 *
 *  ./btrfs-snap-test  test-volume  test-snapshot-1  test-snapshot-2
 */

int main(int argc, char **argv)
{
    int volume_fd;
    int subvolume_fd;
    int snapshot_fd;
    struct btrfs_ioctl_vol_args args = {0};
#ifdef BTRFS_IOC_SNAP_CREATE_V2
    struct btrfs_ioctl_vol_args_v2_new args_v2 = {0};
    struct btrfs_qgroup_inherit inherit = {0};
    struct btrfs_qgroup_limit lim = {0};
#endif
    struct btrfs_ioctl_get_subvol_info_args info = {0};
    char subvolume_path[BTRFS_PATH_NAME_MAX];
    char snapshot_path[BTRFS_PATH_NAME_MAX];

    if (argc < 5) {
        fprintf(stderr, "missing args\n");
        return 1;
    }

    volume_fd = openat(AT_FDCWD, "/mnt", O_RDONLY|O_NONBLOCK
                       |O_CLOEXEC|O_DIRECTORY);

    if (volume_fd < 0) {
        perror("open");
        return 1;
    }

    strncpy(subvolume_path, "/mnt/", BTRFS_PATH_NAME_MAX);
    strncat(subvolume_path, argv[1], BTRFS_PATH_NAME_MAX);

    subvolume_fd = openat(AT_FDCWD, subvolume_path, O_RDONLY|O_NONBLOCK |
                          O_CLOEXEC|O_DIRECTORY);

    args.fd = subvolume_fd;
    strncpy(args.name, argv[2], BTRFS_PATH_NAME_MAX);

    if (ioctl(volume_fd, BTRFS_IOC_SNAP_CREATE, &args) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_SNAP_CREATE:\n");
    printf("Snapshot \"/mnt/%s\" created for subvolume \"%s\"\n\n",
           args.name, subvolume_path);

#ifdef BTRFS_IOC_SNAP_CREATE_V2
    args_v2.fd = subvolume_fd;
    strncpy(args_v2.name, argv[3], BTRFS_PATH_NAME_MAX);
    args_v2.flags = BTRFS_SUBVOL_QGROUP_INHERIT;
    args_v2.size = sizeof(inherit) + sizeof(u_int64_t);
    inherit.lim = lim;
    inherit.num_qgroups = 1;
    args_v2.qgroup_inherit = &inherit;

    if (ioctl(volume_fd, BTRFS_IOC_SNAP_CREATE_V2, &args_v2) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_SNAP_CREATE_V2:\n");
    printf("Snapshot \"/mnt/%s\" created for subvolume \"%s\"\n\n",
           args_v2.name, subvolume_path);

    args_v2.fd = subvolume_fd;
    strncpy(args_v2.name, argv[4], BTRFS_PATH_NAME_MAX);

    if (ioctl(volume_fd, BTRFS_IOC_SNAP_CREATE_V2, &args_v2) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_SNAP_CREATE_V2:\n");
    printf("Snapshot \"/mnt/%s\" created for subvolume \"%s\"\n\n",
           args_v2.name, subvolume_path);
#endif

    if (ioctl(volume_fd, BTRFS_IOC_SNAP_DESTROY, &args) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_SNAP_DESTROY:\n");
    printf("Snapshot \"/mnt/%s\" removed for subvolume \"%s\"\n\n",
           args.name, subvolume_path);

#ifdef BTRFS_IOC_SNAP_CREATE_V2
    args_v2.flags = BTRFS_SUBVOL_SPEC_BY_ID;
    strncpy(snapshot_path, "/mnt/", BTRFS_PATH_NAME_MAX);
    strncat(snapshot_path, argv[3], BTRFS_PATH_NAME_MAX);
    snapshot_fd = openat(AT_FDCWD, snapshot_path, O_RDONLY|O_NONBLOCK |
                          O_CLOEXEC|O_DIRECTORY);

    if (ioctl(snapshot_fd, BTRFS_IOC_GET_SUBVOL_INFO, &info) < 0) {
        perror("ioctl");
        return 1;
    }

    strncpy(args_v2.name, "", BTRFS_PATH_NAME_MAX);
    args_v2.subvolid = info.treeid;

    if (ioctl(volume_fd, BTRFS_IOC_SNAP_DESTROY_V2, &args_v2) < 0) {
        perror("ioctl");
        return -1;
    }
    printf("ioctl BTRFS_IOC_SNAP_DESTROY_V2:\n");
    printf("Snapshot \"/mnt/%s\" removed for subvolume \"%s\"\n\n",
           argv[3], subvolume_path);

    args_v2.flags = 0;
    strncpy(args_v2.name, argv[4], BTRFS_PATH_NAME_MAX);

    if (ioctl(volume_fd, BTRFS_IOC_SNAP_DESTROY_V2, &args_v2) < 0) {
        perror("ioctl");
        return -1;
    }
    printf("ioctl BTRFS_IOC_SNAP_DESTROY_V2:\n");
    printf("Snapshot \"/mnt/%s\" removed for subvolume \"%s\"\n\n",
           argv[4], subvolume_path);
#endif

    return 0;
}
