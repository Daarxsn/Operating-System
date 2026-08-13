#ifndef XYRIS_INIT_H
#define XYRIS_INIT_H

/*
 * ============================================================
 * XyrisOS Kernel Initialization Manager
 * ------------------------------------------------------------
 * Responsible for initializing every kernel subsystem
 * in the correct dependency order.
 * ============================================================
 */

/*
 * Initialize the entire kernel.
 */
void kernel_initialize(void);

#endif