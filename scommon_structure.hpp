#ifndef _SCOMMON_STRUCTURE_HPP
#define _SCOMMON_STRUCTURE_HPP

// non thread safe
template <typename T>
class RingQueue
{
public:
    RingQueue(unsigned int size);
    ~RingQueue();
    RingQueue(const RingQueue& que) = delete;
    RingQueue& operator=(const RingQueue& que) = delete;
    bool Push(const T& element);
    void Pop();
    bool Full();
    bool Empty();
    const T& Front(); // don't call when empty
    const T& Back();
private:
    int front;
    int rear;
    bool isEmpty;
    unsigned int size;
    T* buffer;
};

template <typename T>
RingQueue<T>::RingQueue(unsigned int size) :
    size(size),
    front(0),
    rear(0),
    isEmpty(true)
{
    buffer = new T[size];
}

template <typename T>
RingQueue<T>::~RingQueue()
{
    delete[] buffer;
    buffer = nullptr;
}

template <typename T>
bool RingQueue<T>::Push(const T& element)
{
    if (Full())
        return false;
    buffer[rear] = element;
    rear = (rear + 1) % size;
    isEmpty = false;
}

template <typename T>
void RingQueue<T>::Pop()
{
    if (Empty())
        return;
    int next = (front + 1) % size;
    if (next == rear)
        isEmpty = true;
    front = next;
}

template <typename T>
const T& RingQueue<T>::Front()
{
    return buffer[front];
}

template <typename T>
const T& RingQueue<T>::Back()
{
    return buffer[(rear - 1 + size) % size];
}

template <typename T>
bool RingQueue<T>::Full()
{
    return rear == front && !isEmpty;
}

template <typename T>
bool RingQueue<T>::Empty()
{
    return isEmpty;
}

#endif