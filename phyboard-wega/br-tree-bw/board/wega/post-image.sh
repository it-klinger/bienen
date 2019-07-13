#!/bin/sh


[ -f ${BINARIES_DIR}/barebox-am33xx-phytec-phycore-mlo-256mb.img ] && mv ${BINARIES_DIR}/barebox-am33xx-phytec-phycore-mlo-256mb.img ${BINARIES_DIR}/MLO
[ -f ${BINARIES_DIR}/barebox-am33xx-phytec-phycore.img ] && mv ${BINARIES_DIR}/barebox-am33xx-phytec-phycore.img ${BINARIES_DIR}/barebox.bin
[ -f ${BINARIES_DIR}/barebox-env ] && mv ${BINARIES_DIR}/barebox-env ${BINARIES_DIR}/barebox.env

exit 0

