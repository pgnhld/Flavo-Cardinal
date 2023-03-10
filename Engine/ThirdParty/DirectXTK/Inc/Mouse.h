//--------------------------------------------------------------------------------------
// File: Mouse.h
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
// http://go.microsoft.com/fwlink/?LinkID=615561
//--------------------------------------------------------------------------------------

#pragma once

#include <memory>
#include <Windows.h>

#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
namespace ABI { namespace Windows { namespace UI { namespace Core { struct ICoreWindow; } } } }
#endif


namespace DirectX
{
    class Mouse
    {
    public:
        Mouse();
        Mouse(Mouse&& moveFrom);
        Mouse& operator= (Mouse&& moveFrom);

        Mouse(Mouse const&) = delete;
        Mouse& operator=(Mouse const&) = delete;

        virtual ~Mouse();

        enum Mode
        {
            MODE_ABSOLUTE = 0,
            MODE_RELATIVE,
        };

        struct State
        {
            bool    leftButton;
            bool    middleButton;
            bool    rightButton;
            bool    xButton1;
            bool    xButton2;
            int     x;
            int     y;
            int     scrollWheelValue;
            Mode    positionMode;
        };

        class ButtonStateTracker
        {
        public:
            enum ButtonState
            {
                UP = 0,         // Button is up
                HELD = 1,       // Button is held down
                RELEASED = 2,   // Button was just released
                PRESSED = 3,    // Buton was just pressed
            };

            ButtonState leftButton;
            ButtonState middleButton;
            ButtonState rightButton;
            ButtonState xButton1;
            ButtonState xButton2;

            ButtonStateTracker() { Reset(); }

            void __cdecl Update(const State& state);

            void __cdecl Reset();

            State __cdecl GetLastState() const { return lastState; }

        private:
            State lastState;
        };

        // Retrieve the current state of the mouse
        State __cdecl getState() const;

        // Resets the accumulated scroll wheel value
        void __cdecl ResetScrollWheelValue();

        // Sets mouse mode (defaults to absolute)
        void __cdecl SetMode(Mode mode);

        // Feature detection
        bool __cdecl IsConnected() const;

    #if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP) && defined(WM_USER)
        void __cdecl setWindow(HWND window);
        static void __cdecl processMessage(UINT message, WPARAM wParam, LPARAM lParam);
    #endif

    #if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
        void __cdecl setWindow(ABI::Windows::UI::Core::ICoreWindow* window);
    #ifdef __cplusplus_winrt
        void __cdecl setWindow(Windows::UI::Core::CoreWindow^ window)
        {
            // See https://msdn.microsoft.com/en-us/library/hh755802.aspx
            setWindow(reinterpret_cast<ABI::Windows::UI::Core::ICoreWindow*>(window));
        }
    #endif
        static void __cdecl SetDpi(float dpi);
    #endif

        // Singleton
        static Mouse& __cdecl Get();

    private:
        // Private implementation.
        class Impl;

        std::unique_ptr<Impl> pImpl;
    };
}
