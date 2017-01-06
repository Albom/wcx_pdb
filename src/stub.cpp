void* __cdecl malloc(size_t n)
{
    void* pv = HeapAlloc(GetProcessHeap(), 0, n);
    return pv;
}
void* __cdecl calloc(size_t n, size_t s)
{
    return malloc(n*s);
}
void* __cdecl realloc(void* p, size_t n)
{
    if (p == NULL) return malloc(n);
    return HeapReAlloc(GetProcessHeap(), 0, p, n);
}
void __cdecl free(void* p)
{
    if (p == NULL) return;
    HeapFree(GetProcessHeap(), 0, p);
}
void* __cdecl operator new(size_t n)
{
    return malloc(n);
}
void __cdecl operator delete(void* p)
{
    free(p);
}
