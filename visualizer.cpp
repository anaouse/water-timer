// visualizer.cpp
#define NOMINMAX
#include <windows.h>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <chrono>
#include <format>
#include <algorithm>

#include "SDL3/SDL.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "json.hpp"
using json = nlohmann::json;

struct DisplayRecord {
    int64_t timestamp;
	std::string dateUtf8;
    int elapsedMinutes;
};

static std::string toBeijingTimeString(int64_t timestamp) {
	auto tp = std::chrono::system_clock::time_point(std::chrono::seconds{timestamp});
	tp += std::chrono::hours(8);
	return std::format("{:%Y-%m-%d %H:%M:%S}",floor<std::chrono::seconds>(tp));
}

void runVisualizer(const char* savePath) {
    // 初始化 SDL
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Water Timer Stats", 1200, 400, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    // 初始化 ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 1.5f;  // 字体比例
	io.Fonts->AddFontFromFileTTF("./RobotoMonoNerdFont-Regular.ttf", 25, nullptr, io.Fonts->GetGlyphRangesChineseFull());

    bool running = true;
    std::vector<DisplayRecord> records;

    // 读取 JSON 数据
    {
        std::ifstream in(savePath);
        if (in.is_open()) {
            try {
                json j;
                in >> j;
                for (auto& r : j) {
                    DisplayRecord rec;
                    rec.timestamp = r.value("timestamp", 0);
                    rec.elapsedMinutes = r.value("elapsedMinutes", 0);
                    records.push_back(rec);
                }
            } catch (...) {}
        }
    }
	std::sort(records.begin(), records.end(), [](const DisplayRecord& a, const DisplayRecord& b) {
		return a.timestamp > b.timestamp; // 最近的记录在前
	});
    // 计算统计数据
    int totalMinutes = 0;
	std::set<int64_t> days;
    for (auto& r : records) {
		totalMinutes += r.elapsedMinutes;// 总分钟
		days.insert(r.timestamp / 86400); // 有多少天
		r.dateUtf8 = toBeijingTimeString(r.timestamp); // timestamp变utf8
	}
    int averageMinutes = records.empty() ? 0 : totalMinutes / (int)records.size();

    // 主循环
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL3_ProcessEvent(&e);
            if (e.type == SDL_EVENT_QUIT) running = false;
        }

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // 设置窗口占满整个SDL窗口，并隐藏窗口装饰
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Water Timer Statistics", nullptr,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

        ImGui::Text("Total Records: %d", (int)records.size());
        ImGui::Text("Average Time: %d minutes", averageMinutes);
        ImGui::Text("Days Recorded: %d", (int)days.size());
        ImGui::Separator();
        ImGui::Text("Recent Records:");
        ImGui::BeginChild("records", ImVec2(0, -40), true);
        for (auto& r : records) {
            ImGui::Text(" %s  |  Duration: %d min | %lld",r.dateUtf8.c_str() , r.elapsedMinutes, r.timestamp);
        }
        ImGui::EndChild();

        ImGui::End();

        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    // 清理
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main()
{
    const char* savePath = "./water_timer_records.json";
    runVisualizer(savePath);
    return 0;
}
