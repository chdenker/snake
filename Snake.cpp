#include <SDL2/SDL.h>

#include <cstdio>
#include <cstdlib>

// Constants
const char *GAME_TITLE = "Snake";
const unsigned int PLAYER_WIDTH = 50;
const unsigned int PLAYER_HEIGHT = 100;
const unsigned int FOOD_WIDTH = 50;
const unsigned int FOOD_HEIGHT = 50;

class Screen {
public:
	Screen(unsigned int width, unsigned int height);
	~Screen();

	void draw();
	void clear(int red, int green, int blue);
	void render_rect(int x, int y, int width, int height, int red, int green, int blue);

	inline unsigned int get_width() { return width; }
	inline unsigned int get_height() { return height; }

private:
	SDL_Window *window;
	SDL_Renderer *renderer;
	unsigned int width;
	unsigned int height;
};

struct Input {
	bool left;
	bool right;
	bool restart;
};

class Player {
public:
	Player(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int speed);

	void render(Screen &screen);

	inline Input & get_input_ref() { return input; }

private:
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
	unsigned int speed;
	Input input;
};

class Food {
public:
	Food(unsigned int x, unsigned int y, unsigned int width, unsigned int height);

	void render(Screen &screen);

private:
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
};

Player::Player(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int speed)
: x(x),
  y(y),
  width(width),
  height(height),
  speed(speed)
{
	input.left = false;
	input.right = false;
	input.restart = false;
}

void Player::render(Screen &screen)
{
	screen.render_rect(x, y, width, height, 0, 0, 255); // render player as blue rect
}

Food::Food(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
: x(x),
  y(y),
  width(width),
  height(height)
{}

void Food::render(Screen &screen)
{
	screen.render_rect(x, y, width, height, 255, 0, 0); // render food as red rect
}

void process_input(Input &input, bool &game_running)
{
	SDL_Event e;
	SDL_PollEvent(&e);
	switch (e.type) {
	case SDL_KEYDOWN:
		switch (e.key.keysym.sym) {
		case SDLK_LEFT:
			input.left = true;
			break;
		case SDLK_RIGHT:
			input.right = true;
			break;
		case SDLK_r:
			input.restart = true;
			break;
		case SDLK_ESCAPE:
			game_running = false;
			break;
		}
		break;
	case SDL_QUIT:
		game_running = false;
		break;
	}
}

Screen::Screen(unsigned int width, unsigned int height)
: width(width),
  height(height)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::fprintf(stderr, "screen_init: %s\n", SDL_GetError());
		std::exit(-1);
	}

	window = SDL_CreateWindow(GAME_TITLE, 100, 100, width, height, SDL_WINDOW_SHOWN);
	if (window == nullptr) {
		std::fprintf(stderr, "screen_init: %s\n", SDL_GetError());
		SDL_Quit();
		std::exit(-1);
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == nullptr) {
		SDL_DestroyWindow(window);
		std::fprintf(stderr, "screen_init: %s\n", SDL_GetError());
		SDL_Quit();
		std::exit(-1);
	}
}

Screen::~Screen()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Screen::draw()
{
	SDL_RenderPresent(renderer);
}

void Screen::clear(int red, int green, int blue)
{
	SDL_SetRenderDrawColor(renderer, red, green, blue, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
}

void Screen::render_rect(int x, int y, int width, int height, int red, int green, int blue)
{
	SDL_Rect rect = { x, y, width, height };
	SDL_SetRenderDrawColor(renderer, red, green, blue, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &rect);
}

int main()
{
	Screen screen(640, 480);
	screen.clear(0, 255, 0);

	unsigned int player_x = screen.get_width() / 2;
	unsigned int player_y = screen.get_height() - screen.get_height() / 4;
	Player player(player_x, player_y, PLAYER_WIDTH, PLAYER_HEIGHT, 100);

	Food food(0, 0, FOOD_WIDTH, FOOD_HEIGHT);

	bool game_running = true;
	while (game_running) {
		process_input(player.get_input_ref(), game_running);

		player.render(screen);
		food.render(screen);

		screen.draw();
		SDL_Delay(100);	// TODO Add timing logic
	}
	return 0;
}
