#ifndef QUEUE_H
#define QUEUE_H


#define QUEUE_SIZE 500 //size of queue


/**
* Queue
*/
template <typename T>
class Queue
{
public:


	// Default constructor
	Queue();


	// Destructor
	~Queue();


	/**
	* Push a new an element onto the Queue
	*/
	bool enqueue(T element);


	/**
	* Remove the top element from the Queue
	* Precondition: queue is not empty
	*/
	void dequeue();


	/**
	* Get the top element on the Queue
	* Precondition: queue is not empty
	*/
	T peek(void) const;


	/**
	* Test if the Queue is empty
	*/
	bool isEmpty() const;


	/**
	* Number of element on the Queue
	*/
	int size() const;


	// Remove all elements from the Queue
	void clear(void);


	//for testing purpose (dump)
	void getArray(T tempArray[]) {

		int pos = front;//front position

		//loop and set to array
		for (int i = 0; i < numElements; i++)
		{
			tempArray[i] = array[pos];
			pos = (pos + 1) % QUEUE_SIZE;
		}
	}

private:
	// array of T
	T array[QUEUE_SIZE];

	int front;
	int rear;
	int numElements; //number of elements in queue
};


//
// Constructor
//
template <typename T>
Queue <T>::Queue()
{
	front = 0;
	rear = -1;
	numElements = 0;
}


//
// Destructor
//
template <typename T>
Queue <T>::~Queue(void)
{
	clear();
}


//
// enqueue
// 
template <typename T>
bool Queue <T>::enqueue(T element)
{
	if (numElements == QUEUE_SIZE) //full ?
	{
		return false;
	}


	rear = (rear + 1) % QUEUE_SIZE;
	array[rear] = element;


	numElements++;


	return true;
}


//
// dequeue
//


template <typename T>
void Queue <T>::dequeue(void)
{
	if (numElements == 0)
	{
		return;
	}


	front = (front + 1) % QUEUE_SIZE;


	numElements--;
}


//
// clear
//
template <typename T>
void Queue <T>::clear()
{
	for (int i = 0; i < numElements; i++)
	{
		delete array[i];
	}
	front = 0;
	rear = -1;
	numElements = 0;
}


/**
* Get the top element on the Queue
* Precondition: queue is not empty
*/
template <typename T>
T Queue <T>::peek() const {
	return array[front];
}


/**
* Test if the Queue is empty
*/
template <typename T>
bool Queue <T>::isEmpty() const {
	return numElements == 0;
}


/**
* Number of element on the Queue
*/
template <typename T>
int Queue <T>::size() const {
	return numElements;
}


#endif
