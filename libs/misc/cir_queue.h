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
 * @file        cir_queue.h
 * @brief       Circular queue.
 *
 * @author      DangNhat Pham-Huu <51002279@stu.hcmut.edu.vn>
 */

#ifndef CIR_QUEUE_H_
#define CIR_QUEUE_H_

#include <cstdint>

class cir_queue {
public:

    /**
     * @brief   constructor, user must allocate data for the queue.
     *          Init private (head = 0, tail = -1)
     *
     * @param[in]   queue_p, pointer to buffer for queue.
     * @param[in]   queue_size, size of the buffer.
     *
     * @return  none
     */
    cir_queue(uint8_t *queue_p, uint16_t queue_size);
    
    /**
     * @brief   add one byte to head of the circular queue.
     *
     * @param [in]  a_byte
     *
     * @return  none
     */
    void add_data(uint8_t a_byte);
    
    /**
     * @brief   add a buffer of bytes to head of the circular queue.
     *
     * @param [in]  buf, a buffer of bytes.
     * @param [in]  size, size of the buffer.
     *
     * @return  none
     */
    void add_data(uint8_t* buf, int32_t size);
    
    /**
	 * @brief   preview one byte data from tail of the circular queue.
	 *
	 * @param [in] cont, true if continue from previous point, otherwise, begin from tail.
	 *
	 * @return  data of a byte (0 if queue is empty).
	 */
	uint8_t preview_data(bool cont);

    /**
     * @brief   get one byte from tail of the circular queue.
     *
     * @return  data of a byte (0 if queue is empty).
     */
    uint8_t get_data(void);
    
    /**
     * @brief   get a buffer of bytes from tail of the circular queue.
     *
     * @param [out]  buf, a buffer of bytes.
     * @param [in]  size, size of the buffer.
     *
     * @return  actual number of byte has been read (<= size).
     */
    int32_t get_data(uint8_t* buf, int32_t size);
    
    /**
     * @brief   get size of the circular queue.
     *
     * @return  size
     */
    int32_t get_size(void);
    
    /**
     * @brief   get head of the circular queue.
     *
     * @return  head
     */
    int32_t get_head(void) { return head; }
    
    /**
     * @brief   get tail of the circular queue.
     *
     * @return  tail
     */
    int32_t get_tail(void) { return tail; }
    
    /**
     * @brief   check if queue has been overflowed or not.
     *
     * @return  true if overflowed. Otherwise, false.
     */
    bool is_overflowed(void) { return overflowed; }

protected:
    uint8_t* queue_p;
    uint16_t queue_size;

private:
    int32_t head; /* next pos for new data to be pushed to the queue*/
    int32_t tail; /* next data to be pop from the queue */
    int32_t preview_pos;
    
    /* error indicators */
    bool overflowed;
};

/** @} */
#endif // misc.h
