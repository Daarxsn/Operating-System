void kernel_main(void)
{
    volatile char *video = (volatile char *)0xB8000;

    const char *msg = "XyrisOS Boot Foundation";

    for (int i = 0; msg[i] != '\0'; i++)
    {
        video[i * 2] = msg[i];
        video[i * 2 + 1] = 0x0F;
    }

    while (1)
    {
       cpu_halt_forever();
    }
}