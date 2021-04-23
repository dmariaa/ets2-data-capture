#pragma once

#include <plog/Log.h>
#include <plog/Appenders/IAppender.h>
#include <plog/Init.h>
#include <plog/Formatters/TxtFormatter.h>

#include "imgui/imgui.h"

/// <summary>
/// ImGui PlogD appender
/// Receives PlogD messages, saves them to a buffer, renders log window
/// </summary>
class AppLog : public plog::IAppender
{    
    ImGuiTextBuffer     Buf;
    ImGuiTextFilter     Filter;
    ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
    bool                AutoScroll;  // Keep scrolling if already at the bottom.
    
    plog::TxtFormatter formatter;

public:
    static AppLog* appLog;

    AppLog();
    void Clear();

    void AddLog(const char* fmt, ...) IM_FMTARGS(2);
    void AddLog(std::string str);
    void Draw(bool* p_open = NULL);

    void write(const plog::Record& record) override;
};

/// <summary>
/// ImGui Application settings rendering
/// </summary>
class AppSettings {   
    bool showLogWindow = false;
    bool showImgGuiDemo = false;    

    bool changed = false;
public:
    bool isCapturing = false;
    int currentLogLevel = 0;
    int consecutiveFrames = 1;
    int secondsBetweenSnapshots = 1;

    AppSettings();
    void Draw(const char* windowName = "ETS2DataCapture settings");
    bool hasChanged();
};