/*
 * Copyright (C) 2014 Ho Chi Minh city University of Technology (HCMUT)
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.
 */

/**
 * @defgroup
 * @brief
 * @ingroup     libraries
 * @{
 *
 * @file        cir_queue.cpp
 * @brief       Circular queue.
 *
 * @author      DangNhat Pham-Huu <51002279@stu.hcmut.edu.vn>
 * @}
 */

#include <stddef.h>
#include "cir_queue.h"

/*----------------------------------------------------------------------------*/
cir_queue::cir_queue(uint8_t *queue_p, uint16_t queue_size)
{
    this->queue_p = queue_p;
    this->queue_size = 0;

    this->head = 0;
    this->tail = -1;
    this->preview_pos = this->tail;

    overflowed = false;
}

/*----------------------------------------------------------------------------*/
void cir_queue::add_data(uint8_t a_byte)
{
    int32_t old_head = head;

    queue_p[head] = a_byte;

    if (tail == -1) { /* queue is empty */
        tail = head;
    }

    head = (head + 1) % queue_size;

    /* check overflowed */
    if ((old_head < tail) && (head >= tail)) {
        overflowed = true;
    }
}

/*----------------------------------------------------------------------------*/
void cir_queue::add_data(uint8_t* buf, int32_t size)
{
    int32_t count;

    /* check size */
    if (size < 1) {
        return;
    }

    /* copy data from buffer to queue */
    for (count = 0; count < size; count++) {
        this->add_data(buf[count]);
    }
}

/*----------------------------------------------------------------------------*/
uint8_t cir_queue::preview_data(bool cont)
{
    uint8_t retval;

    if (tail == -1) {
        return 0;
    }

    if (cont == false) {
        preview_pos = tail;
    }

    retval = queue_p[preview_pos];

    preview_pos = (preview_pos + 1) % queue_size;

    return retval;
}

/*----------------------------------------------------------------------------*/
uint8_t cir_queue::get_data(void)
{
    uint8_t retdata;

    if (this->tail == -1) {
        return 0;
    }

    retdata = queue_p[this->tail];
    this->tail = (this->tail + 1) % queue_size;

    if (this->tail == this->head) {
        this->tail = -1;
    }

    return retdata;
}

/*----------------------------------------------------------------------------*/
int32_t cir_queue::get_data(uint8_t* buf, int32_t size)
{
    int32_t queue_size;
    int32_t count;
    int32_t retsize;

    /* check size */
    if (size < 1) {
        return 0;
    }

    queue_size = this->get_size();

    if (queue_size > size) {
        for (count = 0; count < size; count++) {
            buf[count] = this->get_data();
        }

        retsize = size;
    } else {
        for (count = 0; count < queue_size; count++) {
            buf[count] = this->get_data();
        }

        retsize = queue_size;
    }

    return retsize;
}

int32_t cir_queue::get_size(void)
{
    return (tail==-1) ? 0 : (head >= tail ? head - tail : head + queue_size - tail);
}

