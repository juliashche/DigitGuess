#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "NetWork.h"


NetWork NW;

const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 650;
const int FIELD_WIDTH = 400;
const int FIELD_HEIGHT = 400;
const int FIELD_X = (WINDOW_WIDTH - FIELD_WIDTH) / 2 - 150;
const int FIELD_Y = (WINDOW_HEIGHT - FIELD_HEIGHT) / 2 + 50;
const int BUTTON_WIDTH = 110;
const int BUTTON_HEIGHT = 30;
const int BUTTON_X = WINDOW_WIDTH - 400;
const int CLEAR_BUTTON_Y = 170;
const int CHECK_BUTTON_Y = 230;
const int SCALE_FACTOR = 400 / 28; // ������ ���������������

bool isInField(int x, int y) {
    return x >= FIELD_X && x <= FIELD_X + FIELD_WIDTH &&
        y >= FIELD_Y && y <= FIELD_Y + FIELD_HEIGHT;
}

bool isInButton(int x, int y, int buttonY) {
    return x >= BUTTON_X && x <= BUTTON_X + BUTTON_WIDTH &&
        y >= buttonY && y <= buttonY + BUTTON_HEIGHT;
}

void saveImage(const std::vector<std::vector<int>>& imageData) {
    std::ofstream fout("image_data.txt");
    for (int i = 0; i < 28; ++i) {
        for (int j = 0; j < 28; ++j) {
            bool hasDrawing = false;
            // ��������� ��� ������� � �������� 14x14
            for (int k = 0; k < SCALE_FACTOR; ++k) {
                for (int l = 0; l < SCALE_FACTOR; ++l) {
                    if (imageData[i * SCALE_FACTOR + k][j * SCALE_FACTOR + l] == 1) {
                        hasDrawing = true;
                        break;
                    }
                }
                if (hasDrawing) break;
            }
            // ���� ���� ������� ���������, ���������� 1, ����� 0
            fout << (hasDrawing ? 1 : 0) << " ";
        }
        fout << std::endl;
    }
    fout.close();
}

data_NetWork ReadDataNetWork(string path) {
    data_NetWork data{};
    ifstream fin;
    fin.open(path);
    if (!fin.is_open()) {
        cout << "Error reading the file " << path << endl;
        system("pause");
    }
    else
        cout << path << " loading...\n";
    string tmp;
    int L;
    while (!fin.eof()) {
        fin >> tmp;
        if (tmp == "NetWork") {
            fin >> L;
            data.L = L;
            data.size = new int[L];
            for (int i = 0; i < L; i++) {
                fin >> data.size[i];
            }
        }
    }
    fin.close();
    return data;
}

std::string StartGuess(SDL_Renderer* renderer, TTF_Font* font, SDL_Color textColor) {
    double inputData[784]; // ������ ��� �������� ������� ������

    // ������ ������ �� ����� image_data.txt
    std::ifstream fin("image_data.txt");
    if (!fin.is_open()) {
        std::cerr << "Error: Failed to open image data file." << std::endl;
        return "";
    }

    // ���������� ������� inputData ������� �� �����
    for (int i = 0; i < 784; ++i) {
        if (!(fin >> inputData[i])) {
            std::cerr << "Error: Failed to read image data." << std::endl;
            fin.close();
            return "";
        }
    }
    fin.close();
    NW.SetInput(inputData);
    int digit = NW.ForwardFeed();

    // ���������� ������������� ����� � ���� ������
    return std::to_string(digit);
}




int main(int argc, char* argv[]) {
    data_NetWork NW_config;
    NW_config = ReadDataNetWork("Config.txt");
    NW.Init(NW_config);
    NW.ReadWeights();

    std::vector<std::vector<int>> imageData(400, std::vector<int>(400, 0));
    bool quit = false;
    // ������������� SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    // �������� ����
    SDL_Window* window = SDL_CreateWindow("SDL Drawing Example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // �������� ����������� ��� ���������
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // ������������� SDL_ttf ��� ������ � �������
    if (TTF_Init() == -1) {
        std::cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // �������� ������
    TTF_Font* font = TTF_OpenFont("Arial.ttf", 20);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        TTF_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // ���� ������
    SDL_Color textColor = { 255, 255, 255, 255 };

    bool checked = false; // ���������� ��� ������������ ������� ������ "Check"
    // ������� ���� ��������� �������
    while (!quit) {
        // ��������� �������
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                if (isInField(e.motion.x, e.motion.y)) {
                    // ������������� �������� ������� � 1, ���� �� ��������
                    imageData[e.motion.y - FIELD_Y][e.motion.x - FIELD_X] = 1;
                }
                else if (isInButton(e.motion.x, e.motion.y, CLEAR_BUTTON_Y)) {
                    // ������� �����������, ���� ������ ������ Clear
                    imageData.assign(400, std::vector<int>(400, 0));
                    checked = false; // ���������� ���� checked ��� �������
                }
                else if (isInButton(e.motion.x, e.motion.y, CHECK_BUTTON_Y)) {
                    // ��� ������� �� ������ Check ��������� ����������� � ����
                    saveImage(imageData);
                    checked = true; // ������������� ���� checked ��� ������� �� Check
                }
            }
            else if (e.type == SDL_MOUSEMOTION && (e.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT))) {
                if (isInField(e.motion.x, e.motion.y)) {
                    // ������������� �������� ������� � 1, ���� �� ��������
                    imageData[e.motion.y - FIELD_Y][e.motion.x - FIELD_X] = 1;
                }
            }
        }

        // ������� ������
        SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255);
        SDL_RenderClear(renderer);



        // ��������� ���� ��� ���������
        SDL_Rect fieldRect = { FIELD_X, FIELD_Y, FIELD_WIDTH, FIELD_HEIGHT };
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // ������������� ���� ��� ���� (�����)
        SDL_RenderDrawRect(renderer, &fieldRect); // ������ �������������



        // ��������� ������ Clear
        SDL_Rect clearButtonRect = { BUTTON_X, CLEAR_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT };
        SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255); // ������������� ���� ��� ������ (�������)
        SDL_RenderFillRect(renderer, &clearButtonRect); // ������ ������������� ������ Clear

        // ��������� ������ �� ������ Clear
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, "TRY AGAIN", textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect = { BUTTON_X + 5, CLEAR_BUTTON_Y + 5, textSurface->w, textSurface->h };
        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);

        // ��������� ������ Check
        SDL_Rect checkButtonRect = { BUTTON_X, CHECK_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT };
        SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255); // ������������� ���� ��� ������ (�������)
        SDL_RenderFillRect(renderer, &checkButtonRect); // ������ ������������� ������ Check

        // ��������� ������ �� ������ Check
        SDL_Surface* textSurface1 = TTF_RenderText_Solid(font, "CHECK", textColor);
        SDL_Texture* textTexture1 = SDL_CreateTextureFromSurface(renderer, textSurface1);
        SDL_Rect textRect1 = { BUTTON_X + 20, CLEAR_BUTTON_Y + 65, textSurface1->w, textSurface1->h };
        SDL_RenderCopy(renderer, textTexture1, nullptr, &textRect1);
        SDL_FreeSurface(textSurface1);
        SDL_DestroyTexture(textTexture1);

        // ��������� ������� ������
        TTF_Font* font = TTF_OpenFont("arial.ttf", 36); 
        // ��������� ������ ���������
        SDL_Surface* textSurface2 = TTF_RenderText_Solid(font, "Welcome to DigitGuess! ", textColor);
        SDL_Texture* textTexture2 = SDL_CreateTextureFromSurface(renderer, textSurface2);
        SDL_Rect textRect2 = { 100, 70, textSurface2->w, textSurface2->h };
        SDL_RenderCopy(renderer, textTexture2, nullptr, &textRect2);
        SDL_FreeSurface(textSurface2);
        SDL_DestroyTexture(textTexture2);

        // ��������� ������� ������
        TTF_Font* font3 = TTF_OpenFont("arial.ttf", 28);
        // ��������� ������ ���������
        SDL_Surface* textSurface3 = TTF_RenderText_Solid(font3, "Think of any digit, draw it and press CHECK", textColor);
        SDL_Texture* textTexture3 = SDL_CreateTextureFromSurface(renderer, textSurface3);
        SDL_Rect textRect3 = { 100, 120, textSurface3->w, textSurface3->h };
        SDL_RenderCopy(renderer, textTexture3, nullptr, &textRect3);
        SDL_FreeSurface(textSurface3);
        SDL_DestroyTexture(textTexture3);


        // ��������� �����, �������� ��������
        for (int i = 0; i < 400; ++i) {
            for (int j = 0; j < 400; ++j) {
                if (imageData[i][j] == 1) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // ������������� ���� ��� ��������� (�����)
                    SDL_RenderDrawPoint(renderer, FIELD_X + j, FIELD_Y + i); // ������ �����
                }
            }
        }

        // ���� ������ "Check" ���� ������, ���������� ���������
        if (checked) {
            // ���������� ����� � ������������� ������
            // ��������� ������� ������
            TTF_Font* fontt = TTF_OpenFont("arial.ttf", 24);
            std::string predictedDigit = StartGuess(renderer, fontt, textColor);
            SDL_Surface* textSurface = TTF_RenderText_Solid(fontt, ("Have you guessed digit " + predictedDigit + "?").c_str(), textColor);
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_Rect textRect = { BUTTON_X, 300, textSurface->w, textSurface->h };
            SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
            SDL_FreeSurface(textSurface);
            SDL_DestroyTexture(textTexture);

            // ���������� �������������� �����
            SDL_Surface* textSurface1 = TTF_RenderText_Solid(fontt, "If not, please TRY AGAIN", textColor);
            SDL_Texture* textTexture1 = SDL_CreateTextureFromSurface(renderer, textSurface1);
            SDL_Rect textRect1 = { BUTTON_X, 350, textSurface1->w, textSurface1->h };
            SDL_RenderCopy(renderer, textTexture1, nullptr, &textRect1);
            SDL_FreeSurface(textSurface1);
            SDL_DestroyTexture(textTexture1);
        }

        // ���������� ������
        SDL_RenderPresent(renderer);
    }

    // ������������ ��������
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

