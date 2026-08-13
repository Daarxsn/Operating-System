#include "context.h"

#include <stddef.h>

/*
 * ============================================================
 * XyrisOS Context Manager
 * ============================================================
 *
 * This is the process-context API used by the active scheduler.
 *
 * The authoritative x86-64 register save/restore implementation
 * is kernel/process/switch.S. The save/restore helpers remain
 * intentionally small API placeholders until architecture-specific
 * implementations are required outside context_switch().
 */

/*
 * ------------------------------------------------------------
 * Initialize Context Manager
 * ------------------------------------------------------------
 */

void context_initialize(void)
{
    /*
     * Nothing to initialize yet.
     *
     * Future:
     * - Per-CPU context storage
     * - Architecture initialization
     */
}

/*
 * ------------------------------------------------------------
 * Save Context
 * ------------------------------------------------------------
 */

void context_save(context_t *context)
{
    if (context == NULL)
    {
        return;
    }

    /*
     * The active scheduler uses context_switch() for the complete
     * register save/restore operation. This helper is intentionally
     * non-invasive so callers cannot accidentally save a partial
     * context.
     */
}

/*
 * ------------------------------------------------------------
 * Restore Context
 * ------------------------------------------------------------
 */

void context_restore(context_t *context)
{
    if (context == NULL)
    {
        return;
    }

    /*
     * The active scheduler uses context_switch() for the complete
     * register save/restore operation.
     */
}

