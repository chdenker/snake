#include <SDL2/SDL.h>

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>

// Constants
const char *GAME_TITLE = "Snake";
const unsigned int SCREEN_WIDTH = 1024;
const unsigned int SCREEN_HEIGHT = 768;
const unsigned int BPART_WIDTH = 50;
const unsigned int BPART_HEIGHT = 50;
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
	bool up;
	bool down;
	bool restart;
};

struct BodyPart {
	unsigned int x;
	unsigned int y;
	unsigned int next_x;
	unsigned int next_y;
	unsigned int width;
	unsigned int height;
};

class Food {
public:
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;

	Food(unsigned int x, unsigned int y, unsigned int width, unsigned int height);

	void render(Screen &screen);
	void update(unsigned int screen_width, unsigned int screen_height);
};

class Player {
public:
	Player(unsigned int head_x, unsigned int head_y, unsigned int bpart_width, unsigned int bpart_height);

	void render(Screen &screen);
	void update(unsigned int screen_width, unsigned int screen_height);
	void reset_inputs();
	void grow();

	inline Input & get_input_ref() { return input; }
	inline BodyPart & get_head_ref() { return bparts[0]; }
	inline unsigned int get_total_width() { return bparts.size() * bparts[0].width; }
	inline unsigned int get_total_height() { return bparts.size() * bparts[0].height; }

	inline bool head_collided_with(Food &f)
	{
		// TODO Check if this is correct
		return (bparts[0].x == f.x && (bparts[0].y > f.y + f.height || bparts[0].y + bparts[0].height > f.y))
			|| (bparts[0].y == f.y && (bparts[0].x < f.x + f.width  || bparts[0].x + bparts[0].height > f.x));
	}

private:
	std::vector<BodyPart> bparts;
	int direction_x;
	int direction_y;
	Input input;
};

Player::Player(unsigned int head_x, unsigned int head_y, unsigned int bpart_width, unsigned int bpart_height)
:  bparts{ { head_x, head_y, head_x, head_y, bpart_width, bpart_height }, { head_x, head_y + bpart_width, head_x, head_y + bpart_width, bpart_width, bpart_height } },
  direction_x(0),
  direction_y(-1)
{
	reset_inputs();
}

void Player::render(Screen &screen)
{
	// Render all body parts as blue rectangles
	for (BodyPart &bp : bparts) {
		screen.render_rect(bp.x, bp.y, bp.width, bp.height, 0, 0, 255);
	}
}

void Player::reset_inputs()
{
	input.left = false;
	input.right = false;
	input.up = false;
	input.down = false;
	input.restart = false;
}

void Player::grow()
{
	BodyPart &old_tail = bparts.back();
	// TODO Also consider special cases (where snake leaves the screen)
	std::puts("Player::grow");
}

void Player::update(unsigned int screen_width, unsigned int screen_height)
{
	// Set direction according to input
	Input &input = get_input_ref();
	if (input.up && direction_y != 1) {
		direction_x = 0;
		direction_y = -1;
	} else if (input.down && direction_y != -1) {
		direction_x = 0;
		direction_y = 1;
	} else if (input.left && direction_x != 1) {
		direction_x = -1;
		direction_y = 0;
	} else if (input.right && direction_x != -1) {
		direction_x = 1;
		direction_y = 0;
	}
	if (input.restart) {
		// TODO Reinitialize
	}
	reset_inputs();

	// Update the head's position
	BodyPart &head = get_head_ref();
	head.x = head.next_x;
	head.y = head.next_y;
	head.next_x = (head.x + direction_x * head.width) % screen_width;
	int head_next_x = head.x + direction_x * head.width;
	int head_next_y = head.y + direction_y * head.height;
	if (head_next_x < 0) {	// TODO Integer overflow -> undefined behavior?
		head.next_x = screen_width - get_total_width();
	} else {
		head.next_x = head_next_x % screen_width;
	}
	if (head_next_y < 0) {	// TODO Integer overflow -> undefined behavior?
		head.next_y = screen_height - get_total_height();
	} else {
		head.next_y = head_next_y % screen_height;
	}
	
	// Update the other body parts' positions
	for (unsigned int i = 1; i < bparts.size(); i++) {
		bparts[i].x = bparts[i].next_x;
		bparts[i].y = bparts[i].next_y;
		bparts[i].next_x = bparts[i - 1].x;
		bparts[i].next_y = bparts[i - 1].y;	
	}
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

void Food::update(unsigned int screen_width, unsigned int screen_height)
{
	// TODO
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
		case SDLK_UP:
			input.up = true;
			break;
		case SDLK_DOWN:
			input.down = true;
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

void update(Player &player, Food &food, unsigned int screen_width, unsigned int screen_height)
{
	player.update(screen_width, screen_height);
	food.update(screen_width, screen_height);

	if (player.head_collided_with(food)) {
		std::puts("collision");
		// TODO
		// Increase length of player
		player.grow();
		// Reset food
//		food.reset();
		food = Food(rand() % (SCREEN_WIDTH - FOOD_WIDTH), rand() % (SCREEN_HEIGHT - FOOD_HEIGHT), FOOD_WIDTH, FOOD_HEIGHT); // TODO Does this work? If so, why? food is a reference...
	}
}

int main()
{
	Screen screen(SCREEN_WIDTH, SCREEN_HEIGHT);

	unsigned int player_x = screen.get_width() / 2;
	unsigned int player_y = screen.get_height() - screen.get_height() / 4;
	Player player(player_x, player_y, BPART_WIDTH, BPART_HEIGHT);

	std::srand(std::time(nullptr));
//	Food food(rand() % (SCREEN_WIDTH - FOOD_WIDTH), rand() % (SCREEN_HEIGHT - FOOD_HEIGHT), FOOD_WIDTH, FOOD_HEIGHT);
	Food food(0,0, FOOD_WIDTH, FOOD_HEIGHT);

	bool game_running = true;
	while (game_running) {
		process_input(player.get_input_ref(), game_running);

		screen.clear(0, 200, 0);
		player.render(screen);
		food.render(screen);

		update(player, food, screen.get_width(), screen.get_height());

		screen.draw();
		SDL_Delay(50);	// TODO Add timing logic
	}
	return 0;
}
