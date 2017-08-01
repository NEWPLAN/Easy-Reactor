#ifndef __IO_BUFFER_H__
#define __IO_BUFFER_H__

#include <list>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <pthread.h>
#include <ext/hash_map>

struct io_buffer
{
    io_buffer(uint32_t size): 
    capacity(size), length(0), head(0), next(NULL)
    {
        data = new char[size];
        assert(data);
    }

    void clear()
    {
        length = head = 0;
    }

    void adjust()//move data to head
    {
        if (head != 0)
        {
            ::memmove(data, data + head, length);
            head = 0;
        }
    }

    void copy(const io_buffer* other)
    {
        //only copy data to myself
        ::memcpy(data, other->data + other->head, other->length);
        head = 0;
        length = other->length;
    }

    uint32_t capacity;
    uint32_t length;
    uint32_t head;
    io_buffer* next;
    char* data;
};

class buffer_pool
{
public:
    enum MEM_CAP
    {
        u4K   = 4096,
        u16K  = 16384,
        u64K  = 65536,
        u256K = 262144,
        u1M   = 1048576,
        u4M   = 4194304,
        u8M   = 8388608
    };

    static void init()
    {
        _ins = new buffer_pool();
        assert(_ins);
    }

    static buffer_pool* ins()
    {
        pthread_once(&_once, init);
        return _ins;
    }

    io_buffer* alloc(uint32_t N);

    io_buffer* alloc() { return alloc(u4K); }

    void revert(io_buffer* buffer);

private:
    buffer_pool();

    buffer_pool(const buffer_pool&);
    const buffer_pool& operator=(const buffer_pool&);

    typedef __gnu_cxx::hash_map<uint32_t, io_buffer*> pool_t;
    pool_t _pool;
    uint64_t _total_mem;
    static buffer_pool* _ins;
    static pthread_mutex_t _mutex;
    static pthread_once_t _once;
};

class input_buffer
{
public:
    input_buffer(): _buf(NULL) { }

    ~input_buffer() { clear(); }

    int read_data(int fd);

    const char* data() const { return _buf? _buf->data + _buf->head: NULL; }

    const uint32_t length() const { return _buf? _buf->length: 0; }

    void pop(uint32_t len);

    void clear();

private:
    io_buffer* _buf;
};

class output_buffer
{
public:
    ~output_buffer() { clear(); }

    void send_data(const char* data, uint32_t datlen);

    int write_fd(int fd);

    uint32_t length() const;

    void clear();
private:
    std::list<io_buffer*> _buf_lst;
    typedef std::list<io_buffer*>::iterator buff_it;
    typedef std::list<io_buffer*>::const_iterator cbuff_it;
};

#endif
