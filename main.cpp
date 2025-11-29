// main.cpp
#define NOMINMAX
#include <iostream>
#include <windows.h>
#include <cmath>
#include <fstream>
#include <thread>
#include "json.hpp"
#include "SDL3/SDL.h"

// 数据保存的
using json = nlohmann::json;
const char* savePath = "./water_timer_records.json";
void saveRecord(const char* path, int64_t timestamp, int elapsedMinutes)
{
    json record = {
        {"timestamp", timestamp},
        {"elapsedMinutes", elapsedMinutes}
    };

    json all_records;
    // 如果文件存在，先读进来
    std::ifstream in(path);
    if (in.is_open()) {
        try {
            in >> all_records;
        } catch (...) {
            all_records = json::array();
        }
        in.close();
    } 
	else {
        all_records = json::array();
    }

    // 追加新记录
    all_records.push_back(record);

    // 写回文件
    std::ofstream out(path);
    out << all_records.dump(4);
}

struct Button {
	SDL_FRect rect;
	SDL_Color color;
	bool visible;
};

int runApp()
{
		// 初始化整个window
	std::cout << "start debug" << std::endl;
	SDL_Init(SDL_INIT_VIDEO);
	int windowW = 100;
	int windowH = 400;
	SDL_Window* window = SDL_CreateWindow(
		"water_timer",
		windowW, windowH,
		SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_BORDERLESS | SDL_WINDOW_TRANSPARENT
	);
	
	if(!window) {
		std::cout << "fail to create window" << std::endl;
		return 1;
	}
	std::cout <<"window W,H:" << windowW << " " << windowH << std::endl;

	// 去掉APPWINDOW,加上TOOLWINDOW,修改为悬浮窗样式
    HWND hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
	
	LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
	exStyle &= ~WS_EX_APPWINDOW;
	exStyle |= WS_EX_TOOLWINDOW;
	SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);

	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	// 初始化renderer
	SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	if(!SDL_SetWindowOpacity(window, 1.0f)) {
		std::cout << "fail to set opacity" << std::endl;
	}
	
	// 运行相关
	bool running = true;
	// 拖动相关
	bool isDragging = false;
	float dragStartX = 0;
	float dragStartY = 0;
	int windowStartX = 0;
	int windowStartY = 0;
	// 计时相关
	enum class Status {
		Running,
		Paused
	};
	Status windowStatus = Status::Paused;
	Sint32 elapsedMinutes = 0;
	Sint32 maxMinutes = 60 * 6;
	Sint32 updateInterval = 60000; // 60秒更新一次
	Sint32 lastUpdateTime = SDL_GetTicks();
	
	// 画面布局
	Button buttons[5];
	const size_t buttonNum = std::size(buttons);

	const int intervalNum = buttonNum + 1;
	const float interval = windowW * 0.2 / intervalNum;
	const float buttonW = (windowW - (interval * intervalNum)) / buttonNum;
	const float buttonH = windowH * 0.15; // 下方0.15都是按钮,间隔一个0.05是水面开始
	const float waterAreaTop = windowH * 0.05;//顶部间隔0.05
	const float waterAreaHeight = windowH * 0.75;

	for(int i = 0; i < buttonNum; i++) {
		buttons[i].rect = { interval + interval * i + i * buttonW, windowH - buttonH, buttonW, buttonH};
		buttons[i].visible = false;
	}

	buttons[0].color = {0, 200, 200, 255};
    buttons[1].color = {200, 255, 100, 255};
    buttons[2].color = {255, 220, 100, 255};
	buttons[3].color = {72, 61, 139, 255};
    buttons[4].color = {255, 100, 100, 255};


	// 数据保存相关
	int64_t startTimestamp = 0;
	int debug_num = 0;
	while(running) {
		SDL_Event e;
		bool visibility = false;
		Sint32 now = SDL_GetTicks();
		Sint32 timeSinceLastUpdat = now - lastUpdateTime;
		Sint32 waitTime = updateInterval - timeSinceLastUpdat;
		if(waitTime < 0) waitTime = 0;
		debug_num ++;
		if(debug_num % 100 == 0) {
			std::cout << "wait time:" << waitTime <<" "<< "elapsedMinutes:"<< elapsedMinutes << std::endl;
		}
		bool hasEvent = SDL_WaitEventTimeout(&e, waitTime);
		if(hasEvent) {
			do {
				// quit 
				if(e.type == SDL_EVENT_QUIT) running = false;
				// mouse in and show buttons 
				if(e.type == SDL_EVENT_WINDOW_MOUSE_ENTER) {
					visibility = true;
				}
				if(e.type == SDL_EVENT_WINDOW_MOUSE_LEAVE) {
					visibility = false;
				}
				if (e.type == SDL_EVENT_WINDOW_MOUSE_ENTER || e.type == SDL_EVENT_WINDOW_MOUSE_LEAVE) {
					for (auto& b : buttons) {
						b.visible = visibility;
					}
				}
				// click left 
				if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT) {
					std::cout << "click" << std::endl;
					bool clickButton = false;
					float mx = e.button.x;
					float my = e.button.y;
					for (int i = 0; i < buttonNum; ++i) {
						if (buttons[i].visible &&
							mx >= buttons[i].rect.x && mx <= buttons[i].rect.x + buttons[i].rect.w &&
							my >= buttons[i].rect.y && my <= buttons[i].rect.y + buttons[i].rect.h) {
							if(i == 0) {
								windowStatus = Status::Running;
								if(elapsedMinutes == 0) {
									startTimestamp = std::chrono::duration_cast<std::chrono::seconds>(
											std::chrono::system_clock::now().time_since_epoch()).count();
									std::cout << "Start time recorded: " << startTimestamp << std::endl;
								}
							}
							if(i == 1) windowStatus = Status::Paused;
							if(i == 2) {
								windowStatus = Status::Paused;
								if(startTimestamp !=0 && elapsedMinutes != 0) {

									saveRecord(savePath, startTimestamp, elapsedMinutes);
									std::cout << "Saved before clear:" << elapsedMinutes << " minutes\n";
									elapsedMinutes = 0;
									startTimestamp = 0;
								}
							}
							if(i==3) {
								// std::thread([](){
								// 	runVisualizer(savePath);
								// }).detach();
								STARTUPINFOA si = { sizeof(si) };
								PROCESS_INFORMATION pi;
								if (CreateProcessA(
									"visualizer.exe",
									NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
									CloseHandle(pi.hProcess);
									CloseHandle(pi.hThread);
								} 
								else {
									std::cout << "Failed to launch visualizer.exe" << std::endl;
								}
							}
							if(i == 4) {
								running = false;
								if(startTimestamp !=0 && elapsedMinutes != 0) {
									saveRecord(savePath, startTimestamp, elapsedMinutes);
									std::cout << "Saved before exit:" << elapsedMinutes << " minutes\n";
									elapsedMinutes = 0;
									startTimestamp = 0;
								}
							}
							clickButton = true;
						}
					}
					// drag when do not click button 
					if(!clickButton) {
						isDragging = true;
						SDL_GetGlobalMouseState(&dragStartX, &dragStartY);
						SDL_GetWindowPosition(window, &windowStartX, &windowStartY);
						std::cout << "start cursor x,y:" << dragStartX << " " << dragStartY
							<< " "<< "start window x,y:" << windowStartX << " " << windowStartY << std::endl;
					}
				}
				if(e.type == SDL_EVENT_MOUSE_BUTTON_UP && e.button.button == SDL_BUTTON_LEFT) {
					isDragging = false;
				}
				// 计算鼠标的全局delta然后让window也加上delta就是拖动
				if(e.type == SDL_EVENT_MOUSE_MOTION && isDragging) {
					float globalX, globalY;
					SDL_GetGlobalMouseState(&globalX, &globalY);
					int deltaX = (int)(globalX - dragStartX);
					int deltaY = (int)(globalY - dragStartY);
					int newX = windowStartX + deltaX;
					int newY = windowStartY + deltaY;
					SDL_SetWindowPosition(window, newX, newY);
				}
			} while (SDL_PollEvent(&e));	
		}
		// update elapsedMinutes	
		now = SDL_GetTicks();
		if(now - lastUpdateTime > updateInterval) {
			lastUpdateTime = now;
			if(windowStatus == Status::Running && elapsedMinutes < maxMinutes) {
				elapsedMinutes ++;
			}
		}
		
		// background
		SDL_SetRenderDrawColor(renderer, 30, 30, 30, 100);
		SDL_RenderClear(renderer);
		
		// 水面布局与颜色
		float ratio = (float)elapsedMinutes / maxMinutes;
		float waterLevel = ratio * waterAreaHeight;
		float waterY = waterAreaTop + waterAreaHeight - waterLevel;

		int r = (int)(100 + 155 * ratio);
		int g = (int)(200 - 100 * ratio);
		int b = (int)(255 - 50 * ratio);

		SDL_SetRenderDrawColor(renderer, r, g, b, 180);
		float waterX = interval;
		float waterW = windowW - 2 * interval;
		float waterH = ratio * waterAreaHeight; 
		// std::cout << "waterRect x,y,w,h:" << interval <<" "<< waterY << " "<<windowW-2*interval<<" "
		// 	<< waterH << std::endl;
		SDL_FRect waterRect = {waterX, waterY, waterW, waterH};
		SDL_RenderFillRect(renderer, &waterRect);

		// 绘制刻度线
		SDL_SetRenderDrawColor(renderer, 30, 30, 30, 100);
		const float markInterval = waterAreaHeight / 12; // 12个半小时刻度
		
		for(int i = 0; i <= 12; i++) {
			float y = waterAreaTop + i * markInterval;
			float markW = (i % 2 == 0) ? 15 : 8; // 整时较长，半时较短
			SDL_FRect mark = {interval - 2, y - 1, markW, 2};
			SDL_RenderFillRect(renderer, &mark);
		}
		
		// 暂停时显示半透明蓝色框
		if(windowStatus == Status::Paused) {
			SDL_SetRenderDrawColor(renderer, 100, 180, 220, 80);
			SDL_FRect pauseFrame = {interval - 2, waterAreaTop - 2, windowW - 2 * interval + 4, waterAreaHeight + 4};
			SDL_RenderFillRect(renderer, &pauseFrame);
		}

		// buttons 
        for (auto& b : buttons) {
            if (b.visible) {
                SDL_SetRenderDrawColor(renderer, b.color.r, b.color.g, b.color.b, b.color.a);
                SDL_RenderFillRect(renderer, &b.rect);
            }
        }

        SDL_RenderPresent(renderer);
	}
	SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
	return 0;

}


int main()
{
	runApp();
    return 0;
}

