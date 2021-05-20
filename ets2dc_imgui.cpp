#include "ets2dc_imgui.h"

#include <locale>
#include <codecvt>

#include "config.h"

#pragma region AppLog
AppLog* AppLog::appLog = new AppLog();

AppLog::AppLog() 
{
	AutoScroll = true;
	Clear();
}

void AppLog::Clear()
{
	Buf.clear();
	LineOffsets.clear();
	LineOffsets.push_back(0);
}

void AppLog::AddLog(const char* fmt, ...) 
{
    int old_size = Buf.size();

    va_list args;    
    va_start(args, fmt);
    Buf.appendfv(fmt, args);
    va_end(args);

    for (int new_size = Buf.size(); old_size < new_size; old_size++)
        if (Buf[old_size] == '\n')
            LineOffsets.push_back(old_size + 1);
}

void AppLog::AddLog(std::string str)
{
    int old_size = Buf.size();
    Buf.append(str.c_str());

    for (int new_size = Buf.size(); old_size < new_size; old_size++)
        if (Buf[old_size] == '\n')
            LineOffsets.push_back(old_size + 1);
}

void AppLog::Draw(bool* p_open)
{
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("ETS2DataCapture log", p_open))
    {
        ImGui::End();
        return;
    }

    // Options menu
    if (ImGui::BeginPopup("Options"))
    {
        ImGui::Checkbox("Auto-scroll", &AutoScroll);
        ImGui::EndPopup();
    }

    // Main window
    if (ImGui::Button("Options"))
        ImGui::OpenPopup("Options");
    ImGui::SameLine();
    bool clear = ImGui::Button("Clear");
    ImGui::SameLine();
    bool copy = ImGui::Button("Copy");
    ImGui::SameLine();
    Filter.Draw("Filter", -100.0f);

    ImGui::Separator();
    ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    if (clear)
        Clear();
    if (copy)
        ImGui::LogToClipboard();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    const char* buf = Buf.begin();
    const char* buf_end = Buf.end();
    if (Filter.IsActive())
    {
        // In this example we don't use the clipper when Filter is enabled.
        // This is because we don't have a random access on the result on our filter.
        // A real application processing logs with ten of thousands of entries may want to store the result of
        // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
        for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
        {
            const char* line_start = buf + LineOffsets[line_no];
            const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
            if (Filter.PassFilter(line_start, line_end))
                ImGui::TextUnformatted(line_start, line_end);
        }
    }
    else
    {
        // The simplest and easy way to display the entire buffer:
        //   ImGui::TextUnformatted(buf_begin, buf_end);
        // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
        // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
        // within the visible area.
        // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
        // on your side is recommended. Using ImGuiListClipper requires
        // - A) random access into your data
        // - B) items all being the  same height,
        // both of which we can handle since we an array pointing to the beginning of each line of text.
        // When using the filter (in the block of code above) we don't have random access into the data to display
        // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
        // it possible (and would be recommended if you want to search through tens of thousands of entries).
        ImGuiListClipper clipper;
        clipper.Begin(LineOffsets.Size);
        while (clipper.Step())
        {
            for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
            {
                const char* line_start = buf + LineOffsets[line_no];
                const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                ImGui::TextUnformatted(line_start, line_end);
            }
        }
        clipper.End();
    }
    ImGui::PopStyleVar();

    if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();
}

void AppLog::write(const plog::Record& record)
{
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv1;

    plog::util::nstring logString = formatter.format(record);
    // std::string str(logString.begin(), logString.end());
    std::string str = conv1.to_bytes(logString.data());
    AddLog(str);
}
#pragma endregion

AppSettings::AppSettings() 
{
    std::string configLogLevel = ets2dc_config::get(ets2dc_config::keys::log_level, ets2dc_config::default_values::log_level);

    currentLogLevel = ets2dc_utils::LogLevelFromString(configLogLevel);
    consecutiveFrames = ets2dc_config::get(ets2dc_config::keys::consecutive_frames, 2);
    secondsBetweenSnapshots = ets2dc_config::get(ets2dc_config::keys::seconds_between_captures, 3);
    captureDepth = ets2dc_config::get(ets2dc_config::keys::capture_depth, true);
    captureTelemetry = ets2dc_config::get(ets2dc_config::keys::capture_telemtry, true);

    simulate = false;
}

void AppSettings::Draw(const char* windowName)
{
    ImGui::Begin(windowName, NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    {
        bool someChanged = false;
        ImGui::Checkbox("Show log window", &showLogWindow);
        ImGui::SameLine();
        ImGui::Checkbox("Show ImGui demo", &showImgGuiDemo);
        ImGui::Separator();        

        const char* label = ets2dc_utils::logLevels[currentLogLevel].c_str();

        if (ImGui::BeginCombo("Log level", label, 0)) {
            for (int i = 0; i < ets2dc_utils::logLevels.size(); i++) {
                const bool is_selected = (i == currentLogLevel);

                if (ImGui::Selectable(ets2dc_utils::logLevels[i].c_str(), is_selected))
                {
                    currentLogLevel = i;
                    someChanged = true;
                }
                    
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        ImGui::Separator();
        someChanged |= changed || ImGui::SliderInt("FPS recorded", &consecutiveFrames, 1, 30);
        someChanged |= ImGui::SliderInt("Seconds between snapshots", &secondsBetweenSnapshots, 1, 60);
        someChanged |= ImGui::Checkbox("Capture depth", &captureDepth); ImGui::SameLine(); 
        someChanged |= ImGui::Checkbox("Capture telemtry", &captureTelemetry);

        ImGui::Separator();

        someChanged |= ImGui::Checkbox("Simulate (log only)", &simulate);
        if (ImGui::Button(isCapturing ? "Stop capturing" : "Start capturing")) 
        {
            isCapturing = !isCapturing;
            someChanged = true;
        }    

        if (someChanged)
            SaveSettings();

        if(!changed)
            changed = someChanged;
    }
    ImGui::End();

    if (showLogWindow) {
        AppLog::appLog->Draw(&showLogWindow);
    }

    if (showImgGuiDemo) {
        ImGui::ShowDemoWindow();
    }
}

bool AppSettings::hasChanged()
{
    bool hasChanged = changed;
    changed = false;
    return hasChanged;
}

void AppSettings::SaveSettings()
{
    ets2dc_config::begin_save_session();
    ets2dc_config::set(ets2dc_config::keys::consecutive_frames, consecutiveFrames);
    ets2dc_config::set(ets2dc_config::keys::seconds_between_captures, secondsBetweenSnapshots);
    ets2dc_config::set(ets2dc_config::keys::capture_depth, captureDepth);
    ets2dc_config::set(ets2dc_config::keys::capture_telemtry, captureTelemetry);
    ets2dc_config::set(ets2dc_config::keys::log_level, ets2dc_utils::logLevels[currentLogLevel].c_str());
    ets2dc_config::end_save_session();
}