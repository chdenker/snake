#include <SDL2/SDL.h>

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <vector>

// =========
// Constants

const char *GAME_TITLE = "Snake";

// We limit the game to 10 FPS.
// For this, we need to calculate the amount of milliseconds per frame,
// which in the case of 10 FPS is 1000 / 10 = 100.
const Uint32 MS_PER_FRAME = 100;

const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 600;
const unsigned int SCALE = 20;
const unsigned int RECT_WIDTH = SCREEN_WIDTH / SCALE;
const unsigned int RECT_HEIGHT = SCREEN_HEIGHT / SCALE;
// =========

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

	Food();

	void spawn();
	void render(Screen &screen);
};

class Player {
public:
	Player(unsigned int head_x, unsigned int head_y, unsigned int bpart_width, unsigned int bpart_height);

	void reinit(unsigned int head_x, unsigned int head_y, unsigned int bpart_width, unsigned int bpart_height);
	void render(Screen &screen);
	void update(Input &input, unsigned int screen_width, unsigned int screen_height);
	void grow();
	bool head_collided_with(Food &f);
	bool collided_with_himself();
	inline void die() { dead = true; }

	inline BodyPart & get_head_ref() { return bparts[0]; }
	inline unsigned int get_size() { return bparts.size(); }
	inline unsigned int get_total_width() { return bparts.size() * bparts[0].width; }
	inline unsigned int get_total_height() { return bparts.size() * bparts[0].height; }
	inline bool is_dead() { return dead; }

private:
	std::vector<BodyPart> bparts;
	int direction_x;
	int direction_y;
	bool dead;
};

void reset_inputs(Input &input)
{
	input.left = false;
	input.right = false;
	input.up = false;
	input.down = false;
	input.restart = false;
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

Player::Player(unsigned int head_x, unsigned int head_y, unsigned int bpart_width, unsigned int bpart_height)
: bparts{ { head_x, head_y, head_x, head_y, bpart_width, bpart_height }, { head_x, head_y + bpart_height, head_x, head_y + bpart_height, bpart_width, bpart_height } },
  direction_x(0),
  direction_y(-1),
  dead(false)
{}

void Player::reinit(unsigned int head_x, unsigned int head_y, unsigned int bpart_width, unsigned int bpart_height)
{
	bparts.clear();
	BodyPart head = { head_x, head_y, head_x, head_y, bpart_width, bpart_height };
	bparts.push_back(head);
	BodyPart tail = { head_x, head_y + bpart_height, head_x, head_y + bpart_height, bpart_width, bpart_height };
	bparts.push_back(tail);
	direction_x = 0;
	direction_y = -1;
	dead = false;
}

void Player::render(Screen &screen)
{
	// Render head as black rectangle
	BodyPart &head = bparts[0];
	screen.render_rect(head.x, head.y, head.width, head.height, 0, 0, 0);
	for (unsigned int i = 1; i < bparts.size(); i++) {
		BodyPart &bp = bparts[i];
		if (!dead) {
			screen.render_rect(bp.x, bp.y, bp.width, bp.height, 0, 0, 255);
		} else {
			screen.render_rect(bp.x, bp.y, bp.width, bp.height, 0, 0, 0);
		}
	}
}

void Player::grow()
{
	BodyPart &old_tail = bparts.back();
	BodyPart new_tail = { old_tail.x, old_tail.y, old_tail.x, old_tail.y, old_tail.width, old_tail.height };
	bparts.push_back(new_tail);
}

bool Player::head_collided_with(Food &f)
{
	// Taken from https://developer.mozilla.org/en-US/docs/Games/Techniques/2D_collision_detection
	return bparts[0].x < f.x + f.width && bparts[0].x + bparts[0].width > f.x
		&& bparts[0].y < f.y + f.height && bparts[0].y + bparts[0].height > f.y;
}

// TODO Check if this can be done more efficiently
bool Player::collided_with_himself()
{
	// Check for head colliding with any body part
	BodyPart &head = bparts[0];
	for (unsigned int i = 1; i < bparts.size(); i++) {
		BodyPart &bp = bparts[i];
		// Check if head collided with bp
		// Taken from https://developer.mozilla.org/en-US/docs/Games/Techniques/2D_collision_detection
		if (head.x < bp.x + bp.width && head.x + head.width > bp.x
			&& head.y < bp.y + bp.height && head.y + head.height > bp.y)
		{
			return true;
		}
	}
	return false;
}

void Player::update(Input &input, unsigned int screen_width, unsigned int screen_height)
{
	// Set direction according to input
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
	reset_inputs(input);

	// Update the head's position
	// TODO Make sure it is divisible by RECT_WIDTH and RECT_HEIGHT
	BodyPart &head = get_head_ref();
	head.x = head.next_x;
	head.y = head.next_y;
	head.next_x = (head.x + direction_x * head.width) % screen_width;
	int head_next_x = head.x + direction_x * head.width;
	int head_next_y = head.y + direction_y * head.height;
	if (head_next_x < 0) {
		head.next_x = screen_width;
	} else {
		head.next_x = head_next_x % screen_width;
	}
	if (head_next_y < 0) {
		head.next_y = screen_height;
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

Food::Food()
{
	spawn();
}

void Food::spawn()
{
	x = (std::rand() % SCALE) * RECT_WIDTH;
	y = (std::rand() % SCALE) * RECT_HEIGHT;
	width = RECT_WIDTH;
	height = RECT_HEIGHT;
}

void Food::render(Screen &screen)
{
	screen.render_rect(x, y, width, height, 255, 0, 0); // render food as red rect
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

void update(Player &player, Food &food, Input &input, unsigned int screen_width, unsigned int screen_height)
{
	if (input.restart) {
	unsigned int player_x = (std::rand() % SCALE) * RECT_WIDTH;
	unsigned int player_y = (std::rand() % SCALE) * RECT_HEIGHT;
		player.reinit(player_x, player_y, RECT_WIDTH, RECT_HEIGHT);
		reset_inputs(input);
	}
	if (player.is_dead()) {
		return;
	}

	player.update(input, screen_width, screen_height);

	if (player.head_collided_with(food)) {
		player.grow();
		food.spawn();
	}

	if (player.collided_with_himself()) {
		player.die();
		std::printf("Final length: %u\n", player.get_size());
	}
}

int main()
{
	Screen screen(SCREEN_WIDTH, SCREEN_HEIGHT);
	Input input;
	reset_inputs(input);

	unsigned int player_x = (std::rand() % SCALE) * RECT_WIDTH;
	unsigned int player_y = (std::rand() % SCALE) * RECT_HEIGHT;
	Player player(player_x, player_y, RECT_WIDTH, RECT_HEIGHT);

	std::srand(std::time(nullptr));
	Food food;

	bool game_running = true;
	while (game_running) {
		Uint32 start_time_ms = SDL_GetTicks();

		process_input(input, game_running);

		screen.clear(0, 200, 0);
		player.render(screen);
		food.render(screen);

		update(player, food, input, screen.get_width(), screen.get_height());

		screen.draw();

		Uint32 curr_time_ms = SDL_GetTicks();
		// Limit the FPS to 10 (in a very simple manner) to make the game run always at the same pace.
		// If updating and rendering takes longer than MS_PER_FRAME,
		// we have start_time_ms + MS_PER_FRAME < curr_time_ms,
		// which leads to problems.
		SDL_Delay(start_time_ms + MS_PER_FRAME - curr_time_ms);
	}

	return 0;
}
