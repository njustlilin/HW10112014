#ifndef __CIRCULAR_BUFFER
#define __CIRCULAR_BUFFER

/* Implement a circularbuffer library with the following API interface */

/**< Circular Buffer Types */
typedef unsigned char INT8U;
typedef INT8U KeyType ;
typedef struct {  
    INT8U writePointer ; /**< write pointer */  
    INT8U readPointer ;  /**< read pointer */  
    INT8U size;         /**< size of circular buffer */
    INT8U mask;         /**< bit Mask of circular buffer size*/   
    KeyType keys[0];    /**< Element of ciruclar buffer */
} CircularBuffer ; 

inline int CircularBufferIsFull(CircularBuffer* que)
{
	if(que.size == que.mask)
		return 1;
}
inline int CircularBufferIsEmpty(CircularBuffer* que)
{ 
	if(que.size == 0)
		return 1;
}
inline int CircularBufferEnque(CircularBuffer* que, KeyType k)
{ 
	que.keys[writePointer] = k;
	que.writePointer ++;
	if (que.writePointer == que.size)
		que.writePointer = 0;

}
inline int CircularBufferDeque(CircularBuffer* que, KeyType* pK)
{ 
	int temp;
	temp = pK;
	que.keys[temp]=0;
	if(pk<que.size-1) pk++;
	else pk ==0
	

}
inline int CircularBufferPrint(CircularBuffer* que)
{ 
	pintf(que.keys[readPointer]);
}

#endif
