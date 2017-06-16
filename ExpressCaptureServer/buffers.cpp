#include "stdafx.h"
#include <stdint.h>
#include <queue>
#include <deque>
#include <list>
#include <queue>
#include <malloc.h>

using namespace std;
#define BUFSIZE 188
#define BUFNUM  10000

// tx buffer queue
static queue <uint8_t *> m_tx_q;
// spare buffer pool
static queue <uint8_t *> m_po_q;
// Mutexes
static CRITICAL_SECTION mutex_po;
static CRITICAL_SECTION mutex_tx;
//
// Allocate a new buffer
//
uint8_t *alloc_tx_buff(void)
{
    uint8_t *b;
    EnterCriticalSection( &mutex_po );
    if(m_po_q.size() > 0 )
    {
        b = m_po_q.front();
        m_po_q.pop();
    }
    else
    {
        // No available memory so allocate some
        b = (uint8_t *)malloc(BUFSIZE);
    }
	LeaveCriticalSection( &mutex_po );
    return b;
}
//
// Allocate and copy a new buffer
//
uint8_t *alloc_copy_tx_buff(uint8_t *in)
{
    uint8_t *b = alloc_tx_buff();
    if(b != NULL) memcpy(b, in, BUFSIZE );
    return b;
}
//
// Release a buffer
//
void rel_tx_buff(uint8_t *b)
{
    EnterCriticalSection( &mutex_po );
    m_po_q.push(b);
    LeaveCriticalSection( &mutex_po );
}
//
// Post a buffer to the tx queue
//
void post_tx_buff( uint8_t *b)
{
	if(b == NULL) return;
	EnterCriticalSection( &mutex_tx );
    if(m_tx_q.size() < BUFNUM)
        m_tx_q.push(b);
    else
        rel_tx_buff(b);// Queue overflow
	LeaveCriticalSection( &mutex_tx );
}
//
// Get a buffer from the tx queue
//
uint8_t *get_tx_buff(void)
{
    uint8_t *b;
	EnterCriticalSection( &mutex_tx );
    if(m_tx_q.size() > 0 )
    {
        b = m_tx_q.front();
        m_tx_q.pop();
    }
    else
    {
        b = NULL;
    }
	LeaveCriticalSection( &mutex_tx );
    return b;
}
int get_tx_buf_qsize(void)
{
	int len;
	EnterCriticalSection( &mutex_tx );
	len = m_tx_q.size();
	LeaveCriticalSection( &mutex_tx );
	return len;
}
int get_tx_buf_percent(void){
	int param;
	EnterCriticalSection( &mutex_tx );
	param = (m_tx_q.size()*100)/BUFNUM;
	LeaveCriticalSection( &mutex_tx );
	return param;
}
// Empty the buffer queue
void tx_buf_empty(void) {

	EnterCriticalSection(&mutex_tx);
	while (m_tx_q.size() > 0) {
		uint8_t *b = m_tx_q.front();
		m_tx_q.pop();
		m_po_q.push(b);
	}
	LeaveCriticalSection(&mutex_tx);
}
//
// Initialise the buffers
//
void tx_buf_init(void)
{
    // Create the mutex
	InitializeCriticalSection(&mutex_po);
	InitializeCriticalSection(&mutex_tx);
}
