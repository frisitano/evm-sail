/* MMIO contract between the guest and the spike host accelerator device
 * (zkvm/accel-device/accel_device.cc). 8-byte registers at ACCEL_BASE. */
#ifndef ACCEL_MMIO_H
#define ACCEL_MMIO_H
#define ACCEL_BASE   0x40000000UL
#define ACC_R_OP     0   /* operation id (= ACC_id)        */
#define ACC_R_IN     1   /* guest addr of the input bytes  */
#define ACC_R_INLEN  2   /* input length                   */
#define ACC_R_OUT    3   /* guest addr of the output buffer*/
#define ACC_R_GO     4   /* store here triggers the compute*/
#define ACC_R_OUTLEN 5   /* result length (device sets it) */
#define ACC_R_OK     6   /* 1 = success                    */
#endif
