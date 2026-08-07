/* C entry for the direct (no-Rust) ZisK guest link.
 *
 * libziskos.a provides _start, which performs the machine-level bring-up and
 * calls main; runtime init and teardown remain explicit here, mirroring the
 * ziskos Rust entrypoint macro's zkvm_init / user main / zkvm_deinit order. */

extern void zkvm_init(void);
extern void zkvm_deinit(void);
extern int zkvm_start(void);

int main(void)
{
    zkvm_init();
    const int status = zkvm_start();
    zkvm_deinit();
    return status;
}
