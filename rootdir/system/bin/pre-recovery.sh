#!/system/bin/sh

PATH=/system/bin

echo $@ > /cache/recovery/boot
sync

