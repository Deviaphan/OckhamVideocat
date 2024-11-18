#pragma once

class ComponentBase
{
public:
    // *result defaults to 0
    virtual bool HandleWindowMessage(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam,
        LRESULT* result)
    {
        return false;
    }
    virtual ~ComponentBase() { }
};
