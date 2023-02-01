// SPDX-License-Identifier: zlib-acknowledgement

internal void
LinuxMakeQueue(platform_work_queue *Queue, u32 ThreadCount, linux_thread_startup *Startups)
{
    Queue->CompletionGoal = 0;
    Queue->CompletionCount = 0;
    
    Queue->NextEntryToWrite = 0;
    Queue->NextEntryToRead = 0;
    
    uint32 InitialCount = 0;
    sem_init(&Queue->SemaphoreHandle, 0, InitialCount);
    
    for(uint32 ThreadIndex = 0;
        ThreadIndex < ThreadCount;
        ++ThreadIndex)
    {
        linux_thread_startup *Startup = Startups + ThreadIndex;
        Startup->Queue = Queue;
        
        pthread_attr_t Attr;
        pthread_t ThreadID;
        pthread_attr_init(&Attr);
        // TODO(michiel): Check return values
        pthread_attr_setdetachstate(&Attr, PTHREAD_CREATE_DETACHED);
        if (pthread_attr_setstacksize(&Attr, 0x100000))
        {
            fprintf(stderr, "Failed to set the thread stack size to 1MB\n");
        }
        int result = pthread_create(&ThreadID, &Attr, ThreadProc, Startup);
        pthread_attr_destroy(&Attr);
    }
}


// TODO(casey): Does LLVM have real read-specific barriers yet?
#define CompletePreviousReadsBeforeFutureReads asm volatile("" ::: "memory")
#define CompletePreviousWritesBeforeFutureWrites asm volatile("" ::: "memory")
inline uint32 AtomicCompareExchangeUInt32(uint32 volatile *Value, uint32 New, uint32 Expected)
{
    uint32 Result = __sync_val_compare_and_swap(Value, Expected, New);
    
    return(Result);
}
inline u64 AtomicExchangeU64(u64 volatile *Value, u64 New)
{
    u64 Result = __sync_lock_test_and_set(Value, New);
    
    return(Result);
}
inline u64 AtomicAddU64(u64 volatile *Value, u64 Addend)
{
    // NOTE(casey): Returns the original value _prior_ to adding
    u64 Result = __sync_fetch_and_add(Value, Addend);
    
    return(Result);
}
inline u32 GetThreadID(void)
{
    u32 ThreadID;
#if defined(__APPLE__) && defined(__x86_64__)
    asm("mov %%gs:0x00,%0" : "=r"(ThreadID));
#elif defined(__i386__)
    asm("mov %%gs:0x08,%0" : "=r"(ThreadID));
#elif defined(__x86_64__)
    asm("mov %%fs:0x10,%0" : "=r"(ThreadID));
#else
#error Unsupported architecture
#endif
    
    return(ThreadID);
}

