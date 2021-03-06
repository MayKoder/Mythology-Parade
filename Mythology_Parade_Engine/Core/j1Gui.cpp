#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Fonts.h"
#include "j1Input.h"
#include "j1Window.h"
#include "j1Gui.h"
#include "j1Minimap.h"
#include "Console.h"
#include "j1Scene.h"
#include "j1Audio.h"
#include "TooltipData.h"
#include "EntityManager.h"
#include "HUD.h"

j1Gui::j1Gui() : j1Module()
{
	name.append("gui");
	winlose_tex = nullptr;
	atlas_file_name_num_0 = "";
	atlas_file_name_num_1 = "";
	atlas_file_name_num_2 = "";
	UIs.clear();
	atlas_num_0 = nullptr;
	atlas_num_1 = nullptr;
	atlas_num_2 = nullptr;
	cursor_tex = nullptr;
	lockClick = cursor_attack = cursor_move = cursor_heal = false;
	cursor_size = { 0,0 };
	LOG("%s", atlas_file_name_num_0.c_str());
}

// Destructor
j1Gui::~j1Gui()
{}

// Called before render is available
bool j1Gui::Awake(pugi::xml_node& conf)
{
	LOG("Loading GUI atlas");
	bool ret = true;

	atlas_file_name_num_0 = conf.child("atlas_0").attribute("file").as_string("");
	atlas_file_name_num_1 = conf.child("atlas_1").attribute("file").as_string("");
	atlas_file_name_num_2 = conf.child("atlas_2").attribute("file").as_string("");
	active = false;

	return ret;
}

// Called before the first frame
bool j1Gui::Start()
{
	atlas_num_0 = App->tex->Load(atlas_file_name_num_0.c_str());
	atlas_num_1 = App->tex->Load(atlas_file_name_num_1.c_str());
	atlas_num_2 = App->tex->Load(atlas_file_name_num_2.c_str());
	for (int i = 0; i < 10; i++) {
		sfx_UI[i] = 0;
	}
	cursor_tex = App->tex->Load("gui/cursors.png");

	winlose_tex = App->tex->Load("gui/WinLoseBackground.png");

	return true;
}

// Update all guis
bool j1Gui::PreUpdate()
{
	bool mouse = false;
	lockClick = false;
	int count = 0;
	if (App->input->GetMouseButtonDown(1) == KEY_DOWN || App->input->GetMouseButtonDown(1) == KEY_REPEAT) 
	{
		for (int i = UIs.size(); i >= 0 && mouse == false; i--) 
		{
			std::list<UI*>::iterator it = UIs.begin();
			std::advance(it, i);

			if (it._Ptr->_Myval != UIs.end()._Ptr->_Myval) 
			{
				mouse = it._Ptr->_Myval->CheckMouse();
			}
			if (mouse == true)count = i;
		}
	}
	if (mouse == true) 
	{
		std::list<UI*>::iterator it = UIs.begin();
		std::advance(it, count);

		if (it._Ptr->_Myval != UIs.end()._Ptr->_Myval && it._Ptr->_Myval->focus == true)
			it._Ptr->_Myval->Move();
	}
	for(std::list<UI*>::iterator it = UIs.begin(); it != UIs.end(); it++)
	{
		it._Ptr->_Myval->PreUpdate();
	}
	return true;
}

// Called after all Updates
bool j1Gui::Update(float dt)
{
	for (std::list<UI*>::iterator it = UIs.begin(); it != UIs.end(); it++)
	{
		it._Ptr->_Myval->Update(dt);
	}

	return true;
}

// Called after all Updates
bool j1Gui::PostUpdate()
{
	if (App->console->console_active == true) {
		for (std::list<UI*>::iterator it = UIs.begin(); it != UIs.end(); it++)
		{
			if (it._Ptr->_Myval->GetPriority() != 2)
				it._Ptr->_Myval->PostUpdate();
		}
		for (std::list<UI*>::iterator it = UIs.begin(); it != UIs.end(); it++)
		{
			if (it._Ptr->_Myval->GetPriority() == 2)
				it._Ptr->_Myval->PostUpdate();
		}
	}
	else {
		for (std::list<UI*>::iterator it = UIs.begin(); it != UIs.end(); it++)
		{
			it._Ptr->_Myval->PostUpdate();
		}
	}

	iPoint rect_position = App->minimap->WorldToMinimap(-App->render->camera.x, -App->render->camera.y);
	if(App->minimap->active==true)
		App->render->DrawQuad({ rect_position.x, rect_position.y, (int)(App->render->camera.w * App->minimap->scale),(int)(App->render->camera.h * App->minimap->scale) }, 255, 255, 255, 255, 
			false, false);
	if (App->minimap->show_damage_area == true) {
		App->minimap->show_damage_area = false;
		minimap_feedback_timer.Start();
	}
	if(minimap_feedback_timer.ReadSec()<=1)
		App->render->DrawQuad({ App->minimap->damage_area.x - 5,App->minimap->damage_area.y - 5,10,10 }, 255, 255, 0, 255, false, false);


	//Show cursor ------------------------------
	int x, y;
	App->input->GetMousePosition(x, y);
	iPoint p = App->render->ScreenToWorld(x, y);
	SDL_Rect sec = { 0, 0, 22, 32 };
	if (cursor_move == true && App->scene->active == true && App->scene->paused_game == false)
		sec = { 162,0,36,36 };
	if (cursor_attack == true && App->scene->active == true && App->scene->paused_game == false)
		sec = { 216,0,35,33 };
	if (cursor_heal == true && App->scene->active == true && App->scene->paused_game == false)
		sec = { 324,0,27,32 };

	cursor_size = { sec.w,sec.h };
	p = App->render->ScreenToWorld(x, y);

	App->render->Blit(cursor_tex, p.x, p.y, &sec);

	//Win or Lose Window
	if (App->entityManager->getPlayer() != nullptr && App->entityManager->active==true) {
		if (App->entityManager->getPlayer()->player_win == true) {
			if (App->scene->isInTutorial == true) {
				DoWinOrLoseWindow(3, true);
			}
			else {
				if (App->entityManager->getPlayer()->player_type == CivilizationType::VIKING) {
					DoWinOrLoseWindow(1, true);
				}
				else {
					DoWinOrLoseWindow(2, true);
				}
			}
		}

		else if (App->entityManager->getPlayer()->player_lose == true) {
			App->entityManager->initCivilizations = true;
			if (App->entityManager->getPlayer()->player_type == CivilizationType::VIKING) {
				DoWinOrLoseWindow(1, false);
			}
			else {
				DoWinOrLoseWindow(2, false);
			}
		}
	}


	return true;
}

// Called before quitting
bool j1Gui::CleanUp()
{
	LOG("Freeing GUI");
	for (std::list<UI*>::iterator it = UIs.begin(); it != UIs.end(); it++)
	{
		it._Ptr->_Myval->CleanUp();
		delete it._Ptr->_Myval;
	}
	UIs.clear();

	if (atlas_num_0)
	{
		App->tex->UnLoad(atlas_num_0);
	}
	if (atlas_num_1)
	{
		App->tex->UnLoad(atlas_num_1);
	}
	if (atlas_num_2)
	{
		App->tex->UnLoad(atlas_num_2);
	}
	App->tex->UnLoad(cursor_tex);
	App->tex->UnLoad(winlose_tex);
	return true;
}

// const getter for atlas
const SDL_Texture* j1Gui::GetAtlas(int number_atlas) const
{
	if (number_atlas == 0)
		return atlas_num_0;
	else if (number_atlas == 1)
		return atlas_num_1;
	else if (number_atlas == 2)
		return atlas_num_2;
}

// class Gui ---------------------------------------------------

UI* j1Gui::CreateUIElement(Type type, UI* p, SDL_Rect r, SDL_Rect sprite, std::string str, Panel_Fade p_fade, SDL_Rect sprite2, SDL_Rect sprite3, bool drageable, SDL_Rect drag_area, j1Module* s_listener, int audio,
	bool console, float drag_position_scroll_bar, int number_atlas, int num_tooltip, bool tooltip_immediate)
{
	UI* ui = nullptr;
	SDL_Color colour;
	bool title;
	switch (type)
	{
	case Type::BUTTON:
		ui = new ButtonUI(Type::BUTTON, p, r, sprite, sprite2, sprite3, true, true, drag_area, audio, p_fade, num_tooltip, tooltip_immediate);
		break;
	case Type::IMAGE:
		ui = new ImageUI(Type::IMAGE, p, r, sprite, drageable, drageable, drag_area, drag_position_scroll_bar, p_fade, num_tooltip, tooltip_immediate);
		break;
	case Type::WINDOW:
		ui = new WindowUI(Type::WINDOW, p, r, sprite, drageable, drageable, drag_area, p_fade);
		break;
	case Type::TEXT:
		colour = { (Uint8)sprite2.x,(Uint8)sprite2.y,(Uint8)sprite2.w,(Uint8)sprite2.h };
		if (sprite3.x == 0)
			title = false;
		else
			title = true;
		ui = new TextUI(Type::TEXT, p, r, str, drageable, drageable, drag_area, console, colour, title, p_fade, num_tooltip, tooltip_immediate);
		break;
	case Type::LISTTEXTS:
		ui = new ListTextsUI(Type::LISTTEXTS, p, r, str, drageable, drageable, drag_area, console, p_fade);
		break;
	}

	ui->active = true;
	ui->name = str.c_str();
	ui->num_atlas = number_atlas;

	if (s_listener)
	{
		ui->listener = s_listener;
	}
	else
	{
		ui->listener = nullptr;
	}


	//UIs.add(ui);

	UIs.push_back(ui);
	return ui;
}

UI* j1Gui::CreateUIElement(Type type, UI* p, SDL_Rect r, std::string str, int re, int g, int b, int a, bool drageable, SDL_Rect drag_area, j1Module* s_listener, Panel_Fade p_fade)
{
	UI* ui = nullptr;
	switch (type)
	{
	case Type::IMAGE:
		ui = new ImageUI(Type::IMAGE, p, r, re, g, b, a, drageable, drageable, drag_area, p_fade);
		break;
	case Type::INPUT:
		ui = new TextInputUI(Type::INPUT, p, r, re, g, b, a, "", drageable, true, drag_area, p_fade);
		break;
	}

	ui->active = true;
	ui->name = str.c_str();

	if (s_listener)
	{
		ui->listener = s_listener;
	}
	else
	{
		ui->listener = nullptr;
	}


	//UIs.add(ui);

	UIs.push_back(ui);
	return ui;
}

bool j1Gui::DeleteUIElement(UI* ui) 
{

	std::list<UI*>::iterator it = UIs.begin();
	it = std::find(UIs.begin(), UIs.end(), ui);

	if (it == UIs.end())return false;
	else
	{
		it._Ptr->_Myval->CleanUp();
		UIs.remove(ui);
		delete it._Ptr->_Myval;
		return true;
	}
}

void j1Gui::ChangeDebug() 
{
	for (std::list<UI*>::iterator it = UIs.begin(); it != UIs.end(); it++)
	{
		it._Ptr->_Myval->debug = !it._Ptr->_Myval->debug;
	}
}

void j1Gui::ChangeFocus() 
{
	bool exit = false;
	bool focus = false;
	int count = 0;
	for (std::list<UI*>::iterator it = UIs.begin(); it != UIs.end() && exit == false; it++)
	{

		bool focusable = it._Ptr->_Myval->CheckFocusable();
		if (focusable == true) {
			count++;
			if (focus == true) {
				it._Ptr->_Myval->focus = true;
				exit = true;
			}
			else {
				focus = it._Ptr->_Myval->focus;
				it._Ptr->_Myval->focus = false;
			}
		}
	}
	if (count > 0 && exit == false) 
	{
		for (std::list<UI*>::iterator it = UIs.begin(); it != UIs.end() && exit == false; it++)
		{
			bool focusable = it._Ptr->_Myval->CheckFocusable();
			if (focusable == true) {
				it._Ptr->_Myval->focus = true;
				exit = true;
			}
		}
	}
}

void j1Gui::DeleteFocus() {
	for (std::list<UI*>::iterator it = UIs.begin(); it != UIs.end(); it++)
	{
		it._Ptr->_Myval->focus = false;
	}
}

void j1Gui::ClearUI()
{
	UIs.clear();
}

void j1Gui::ReturnConsole() {
	if (App->console->console_active == true) {
		App->console->ActivateConsole();
	}
}

void j1Gui::ActivateButtons() {
	for (std::list<UI*>::iterator it = UIs.begin(); it != UIs.end(); it++)
	{
		if (it._Ptr->_Myval->type == Type::BUTTON) {
			ButtonUI* button = (ButtonUI*)it._Ptr->_Myval;
			button->front = true;
		}
	}
}

SDL_Texture* j1Gui::GetTexture()
{
	return atlas_num_0;
}

void j1Gui::DeactivateButtons() {
	for (std::list<UI*>::iterator it = UIs.begin(); it != UIs.end(); it++)
	{
		if (it._Ptr->_Myval->type == Type::BUTTON) {
			ButtonUI* button = (ButtonUI*)it._Ptr->_Myval;
			button->front = false;
		}
	}
}

void j1Gui::WorkWithTextInput(std::string text) {
	bool exit = false;
	for (std::list<UI*>::iterator it = UIs.begin(); it != UIs.end() && exit == false; it++)
	{
		if (it._Ptr->_Myval->type == Type::INPUT &&it._Ptr->_Myval->focus == true)
		{
			TextInputUI* text_ui = (TextInputUI*)it._Ptr->_Myval;
			text_ui->ChangeLabel(text);
		}
	}
}

UI::UI(Type s_type, SDL_Rect r, UI* p, bool d, bool f, SDL_Rect d_area, bool consol, int num_tooltip, bool tooltip_immediate)
{
	name.append("UI");
	type = s_type;
	drageable = d;
	focusable = f;
	screen_rect = { r.x,r.y,r.w,r.h };
	parent = p;
	if (parent != nullptr) {
		local_rect = { screen_rect.x - parent->screen_rect.x, screen_rect.y - parent->screen_rect.y, screen_rect.w, screen_rect.h };
	}
	else {
		local_rect = screen_rect;
	}
	mask_rect = screen_rect;
	debug = false;
	focus = false;
	drag_area = d_area;
	console = consol;
	priority = 1;
	num_atlas = 0;
	if (num_tooltip != -1) {
		has_tooltip = true;
		tooltip_num = num_tooltip;
	}
	else {
		has_tooltip = false;
		tooltip_num = -1;
	}
	for (int i = 0; i < 13; i++) {
		tooltip_texts[i] = nullptr;
	}
	tooltip_window = nullptr;
	has_timer_tooltip_started = false;
	if (tooltip_immediate == true)
		time_tooltip = 0;
	else
		time_tooltip = 1.2;
}

bool UI::PreUpdate() {
	UI* ui = this;



	screen_rect.x = local_rect.x;
	screen_rect.y = local_rect.y;
	while (ui->parent != nullptr) {
		screen_rect.x += ui->parent->local_rect.x;
		screen_rect.y += ui->parent->local_rect.y;
		ui = ui->parent;
	}

	uint win_x, win_y;
	App->win->GetWindowSize(win_x, win_y);
	mask_rect = screen_rect;

	if (parent != nullptr) {
		if (mask_rect.x < parent->mask_rect.x) {
			mask_rect.x += parent->mask_rect.x - mask_rect.x;
			mask_rect.w -= parent->mask_rect.x - mask_rect.x;
		}
		else if (mask_rect.x + mask_rect.w > parent->mask_rect.x + parent->mask_rect.w) {
			mask_rect.w -= mask_rect.x + mask_rect.w - parent->mask_rect.x - parent->mask_rect.w;
		}
		if (mask_rect.y < parent->mask_rect.y) {
			mask_rect.y += parent->mask_rect.y - mask_rect.y;
			mask_rect.h -= parent->mask_rect.y - mask_rect.y;
		}
		else if (mask_rect.y + mask_rect.h > parent->mask_rect.y + parent->mask_rect.h) {
			mask_rect.h -= mask_rect.y + mask_rect.h - parent->mask_rect.y - parent->mask_rect.h;
		}
	}
	else {
		if (mask_rect.x < 0) {
			mask_rect.w -= mask_rect.x;
			mask_rect.x = 0;
		}
		else if (mask_rect.x + mask_rect.w > win_x) {
			mask_rect.w -= mask_rect.x + mask_rect.w - win_x;
		}
		if (mask_rect.y < 0) {
			mask_rect.h -= mask_rect.y;
			mask_rect.y = 0;
		}
		else if (mask_rect.y + mask_rect.h > win_y) {
			mask_rect.h -= mask_rect.y + mask_rect.h - win_y;
		}
	}

	if (has_tooltip == true) {
		int x, y, x_motion, y_motion;
		App->input->GetMouseMotion(x_motion, y_motion);
		if (x_motion == 0 && y_motion == 0) {
			App->input->GetMousePosition(x, y);
			if (x >= screen_rect.x && x <= screen_rect.x + screen_rect.w && y >= screen_rect.y && y <= screen_rect.y + screen_rect.h) {
				if (has_timer_tooltip_started == false) {
					timer_tooltip.Start();
					has_timer_tooltip_started = true;
				}
				else if (timer_tooltip.ReadSec() >= time_tooltip && tooltip_window == nullptr && has_timer_tooltip_started == true) {
					ShowTooltip(x, y, win_x, win_y);
				}
			}
		}

		else if (tooltip_window != nullptr) {
			App->input->GetMousePosition(x, y);
			if (x < screen_rect.x || x > screen_rect.x + screen_rect.w || y < screen_rect.y || y > screen_rect.y + screen_rect.h) {
				DestroyTooltip();
				has_timer_tooltip_started = false;
			}
		}
		else
			has_timer_tooltip_started = false;
	}
	return true;
}

//This Update changes alpha if FADE is active/true
bool UI::Update(float dt) {

	if (fade_panel == Panel_Fade::panel_fade_in) {
		float normalized = MIN(1.0f, (float)fade_panel_timer.ReadSec() / (float)fade_panel_time);
		alpha = normalized * 255;
		if (fade_panel_timer.ReadSec() >= fade_panel_time) {
			alpha = 255;
			fade_panel = Panel_Fade::no_one_fade;
		}
	}

	else if (fade_panel == Panel_Fade::panel_fade_out) {
		float normalized = MIN(1.0f, (float)fade_panel_timer.ReadSec() / (float)fade_panel_time);
		alpha = 255 - (normalized * 255);
		if (fade_panel_timer.ReadSec() >= fade_panel_time) {
			alpha = 255;
			fade_panel = Panel_Fade::no_one_fade;
		}
	}

	return true;
}

bool UI::PostUpdate() {
	if (debug == true) {
		App->render->DrawQuad(screen_rect, 255, 0, 0, 255, false, false);
	}
	return true;
}

void UI::ShowTooltip(int mouse_x, int mouse_y, uint win_x, uint win_y)
{
	Tooltip tooltip = App->tooltipdata->GetTooltip(tooltip_num);
	int x, y;
	if (mouse_x + 322 + App->gui->cursor_size.x <= win_x) {
		x = mouse_x + 5 + App->gui->cursor_size.x;
	}
	else {
		x = mouse_x - 322;
	}
	if (mouse_y + (tooltip.lines*18) <= win_y) {
		y = mouse_y;
	}
	else {
		y = mouse_y - (tooltip.lines * 18);
	}
	tooltip_window = static_cast<WindowUI*>(App->gui->CreateUIElement(Type::WINDOW, nullptr, { x,y,317,(tooltip.lines * 18) }, { 1285,11,305,(tooltip.lines * 18)} ));
	int j = 0;
	for (int i = 1; i <= tooltip.lines; i++) {
		if (tooltip.has_title&& i == 1) {
			tooltip_texts[i - 1] = static_cast<TextUI*>(App->gui->CreateUIElement(Type::TEXT, tooltip_window, { x,y + (18 * (i - 1)),317,18 }, { 0,0,0,0 }, tooltip.title, Panel_Fade::no_one_fade, { 255,255,255,255 }));
			j--;
		}
		else {
			tooltip_texts[i - 1] = static_cast<TextUI*>(App->gui->CreateUIElement(Type::TEXT, tooltip_window, { x,y + (18 * (i - 1)),317,18 }, { 0,0,0,0 },
				App->tooltipdata->GetLineTooltip(i + j, tooltip), Panel_Fade::no_one_fade, { 255,255,255,255 }));
		}
	}	
}

SDL_Rect UI::GetScreenRect()
{
	return screen_rect;
}
SDL_Rect UI::GetParentScreenRect()
{
	if (parent != nullptr)
		return parent->screen_rect;
	else {
		uint width, height;
		App->win->GetWindowSize(width, height);
		return { 0,0,(int)width,(int)height };
	}
}
SDL_Rect UI::GetLocalRect() {
	return local_rect;
}
iPoint UI::GetScreenPos() {
	return { screen_rect.x,screen_rect.y };
}
iPoint UI::GetScreenToWorldPos() {
	uint scale = App->win->GetScale();
	return { screen_rect.x / (int)scale,screen_rect.y / (int)scale };
}
iPoint UI::GetLocalPos() {
	return { local_rect.x,local_rect.y };
}
void UI::SetLocalPos(iPoint pos) {
	iPoint r = { -local_rect.x + pos.x,-local_rect.y + pos.y };
	local_rect.x = pos.x;
	local_rect.y = pos.y;
	screen_rect.x += r.x;
	screen_rect.y += r.y;
}

void UI::SetRect(SDL_Rect rect) {
	screen_rect = rect;
	if (parent != nullptr) {
		local_rect = { screen_rect.x - parent->screen_rect.x, screen_rect.y - parent->screen_rect.y, screen_rect.w, screen_rect.h };
	}
	else {
		local_rect = screen_rect;
	}
	mask_rect = screen_rect;
}

void UI::SetScreenRect(SDL_Rect rect) {
	screen_rect = rect;
}

void UI::UpdateLocalRect() {
	if (parent != nullptr) {
		local_rect = { screen_rect.x - parent->screen_rect.x, screen_rect.y - parent->screen_rect.y, screen_rect.w, screen_rect.h };
	}
	else {
		local_rect = screen_rect;
	}
}

bool UI::CheckMouse() 
{
	if (drageable == true) 
	{
		int x, y;
		App->input->GetMousePosition(x, y);
		if ((x >= screen_rect.x && x <= screen_rect.x + screen_rect.w && y >= screen_rect.y && y <= screen_rect.y + screen_rect.h) || focus == true)
			return true;
	}
	return false;
}

bool UI::CleanUp()
{
	DestroyTooltip();
	return true;
}

void UI::DestroyTooltip()
{
	if (tooltip_window != nullptr) {
		App->gui->DeleteUIElement(tooltip_window);
		tooltip_window = nullptr;
	}
	for (int i = 12; i >= 0; i--) {
		if (tooltip_texts[i] != nullptr) {
			App->gui->DeleteUIElement(tooltip_texts[i]);
			tooltip_texts[i] = nullptr;
		}
	}
}

bool UI::Move() {
	int x, y;
	App->input->GetMouseMotion(x, y);
	x *= App->win->GetScale();
	y *= App->win->GetScale();
	if (screen_rect.x + x >= drag_area.x && screen_rect.x + screen_rect.w + x <= drag_area.x + drag_area.w) {
		local_rect.x += x;
		quad.x += x;
	}
	else if (screen_rect.y + y >= drag_area.y && screen_rect.y + screen_rect.h + y <= drag_area.y + drag_area.h) {
		local_rect.y += y;
		quad.y += y;
	}
	return true;
}

void UI::SetPriority(int prior) {
	priority = prior;
}

SDL_Rect UI::Check_Printable_Rect(SDL_Rect sprite, iPoint& dif_sprite) {
	if (mask_rect.x > screen_rect.x) {
		dif_sprite.x = mask_rect.x - screen_rect.x;
		sprite.x += dif_sprite.x;
		sprite.w -= dif_sprite.x;
	}
	else if (mask_rect.w < screen_rect.w) {
		sprite.w -= screen_rect.w - mask_rect.w;
	}
	if (mask_rect.y > screen_rect.y) {
		dif_sprite.y = mask_rect.y - screen_rect.y;
		sprite.y += dif_sprite.y;
		sprite.h -= dif_sprite.y;
	}
	else if (mask_rect.h < screen_rect.h) {
		sprite.h -= screen_rect.h - mask_rect.h;
	}
	return sprite;
}

SDL_Rect UI::Check_Printable_Rect(SDL_Rect sprite, iPoint& dif_sprite, SDL_Rect quad) {
	SDL_Rect parent_screen_rect = GetParentScreenRect();
	if (mask_rect.x > quad.x) {
		dif_sprite.x = mask_rect.x - quad.x;
		sprite.x += dif_sprite.x;
		sprite.w -= dif_sprite.x;
	}
	else if (mask_rect.w < quad.w) {
		sprite.w -= quad.w - mask_rect.w;
	}
	if (parent_screen_rect.y > quad.y) {
		dif_sprite.y = parent_screen_rect.y - quad.y;
		sprite.y += dif_sprite.y;
		sprite.h -= dif_sprite.y;
	}
	else if (parent_screen_rect.y + parent_screen_rect.h < quad.y + quad.h) {
		sprite.h -= quad.y + quad.h - parent_screen_rect.y - parent_screen_rect.h;
	}
	return sprite;
}


///////////// IMAGE //////////////

ImageUI::ImageUI(Type type, UI * p, SDL_Rect r, SDL_Rect sprite, bool d, bool f, SDL_Rect d_area, float drag_position_scroll_bar, Panel_Fade p_fade, int num_tooltip, bool tooltip_immediate)
	:UI(type, r, p, d, f, d_area, false, num_tooltip, tooltip_immediate) {
	name.append("ImageUI");
	sprite1 = sprite;
	quad = r;
	SDL_Rect drag_area = GetDragArea();
	drag_position_0 = { drag_area.x, drag_area.y };
	drag_position_1 = { drag_area.w + drag_area.x - GetLocalRect().w,drag_area.h + drag_area.y - GetLocalRect().h };
	square = false;
	red = green = blue = alpha = 0;
	unclicked = false;
	fade_panel = p_fade;
	if (drag_position_scroll_bar != -1) {
		quad.x = drag_position_0.x + (drag_position_scroll_bar * (drag_position_1.x - drag_position_0.x));
		SetScreenRect(quad);
		UpdateLocalRect();
	}
	fade_panel = p_fade;

	if (fade_panel == Panel_Fade::no_one_fade) {
		alpha = 255;
	}
	if (fade_panel == Panel_Fade::panel_fade_in) {
		alpha = 0;
		fade_panel_timer.Start();
		fade_panel_time = 1;
	}
}

ImageUI::ImageUI(Type type, UI* p, SDL_Rect r, int re, int g, int b, int a, bool d, bool f, SDL_Rect d_area, Panel_Fade p_fade) :UI(type, r, p, d, f, d_area, true) {
	name.append("ImageUI");
	sprite1 = { 0,0,0,0 };
	quad = r;
	SDL_Rect drag_area = GetDragArea();
	drag_position_0 = { drag_area.x, drag_area.y };
	drag_position_1 = { drag_area.w + drag_area.x - GetLocalRect().w,drag_area.h + drag_area.y - GetLocalRect().h };
	square = true;
	red = re;
	green = g;
	blue = b;
	fade_panel = p_fade;

	if (fade_panel == Panel_Fade::no_one_fade) {
		alpha = a;
	}
	if (fade_panel == Panel_Fade::panel_fade_in) {
		alpha = 0;
		fade_panel_timer.Start();
		fade_panel_time = 1;
	}
}

bool ImageUI::PreUpdate() {
	unclicked = false;
	int x, y;
	iPoint initial_position = GetScreenPos();
	App->input->GetMousePosition(x, y);
	if (CheckFocusable() == true && (x >= GetScreenPos().x && x <= GetScreenPos().x + GetScreenRect().w && y >= GetScreenPos().y && y <= GetScreenPos().y + GetScreenRect().h)) {
		if (App->input->GetMouseButtonDown(1) == KEY_DOWN) {
			App->gui->DeleteFocus();
			focus = true;
		}
	}
	if (focus == true && App->input->GetMouseButtonDown(1) == KEY_UP) {
		focus = false;
		unclicked = true;

	}
	UI::PreUpdate();
	if (initial_position != GetScreenPos()) {
		fPoint drag_position = GetDragPositionNormalized();
		if (listener)
		{
			listener->OnClick(this, drag_position.x);
		}
	}
	return true;
}

bool ImageUI::Update(float dt) {
	UI::Update(dt);

	return true;
}

bool ImageUI::PostUpdate() {
	iPoint dif_sprite = { 0,0 };
	if (square == false) {
		SDL_Rect sprite = UI::Check_Printable_Rect(sprite1, dif_sprite);
		quad.x = GetScreenPos().x + dif_sprite.x;
		quad.y = GetScreenPos().y + dif_sprite.y;
		SDL_SetTextureAlphaMod((SDL_Texture*)App->gui->GetAtlas(num_atlas), alpha);
		if (this->active) App->render->BlitInsideQuad((SDL_Texture*)App->gui->GetAtlas(num_atlas), sprite, quad);
	}
	else if (this->active) {
		App->render->DrawQuad(quad, red, green, blue, alpha, true, false);
	}
	UI::PostUpdate();
	return true;
}

fPoint ImageUI::GetDragPositionNormalized() {
	fPoint position_normalized;
	position_normalized.x = GetScreenPos().x - drag_position_0.x;
	position_normalized.y = GetScreenPos().y - drag_position_0.y;
	position_normalized.x /= drag_position_1.x - drag_position_0.x;
	position_normalized.y /= drag_position_1.y - drag_position_0.y;
	return position_normalized;
}

///////////// WINDOW //////////////

WindowUI::WindowUI(Type type, UI* p, SDL_Rect r, SDL_Rect sprite, bool d, bool f, SDL_Rect d_area, Panel_Fade p_fade) :UI(type, r, p, d, f, d_area) {
	name.append("WindowUI");
	sprite1 = sprite;
	quad = r;
	fade_panel = p_fade;

	if (fade_panel == Panel_Fade::no_one_fade) {
		alpha = 255;
	}
	if (fade_panel == Panel_Fade::panel_fade_in) {
		alpha = 0;
		fade_panel_timer.Start();
		fade_panel_time = 1;
	}
}

bool WindowUI::Update(float dt) {
	UI::Update(dt);

	return true;
}

bool WindowUI::PostUpdate() {
	iPoint dif_sprite = { 0,0 };
	SDL_Rect sprite = UI::Check_Printable_Rect(sprite1, dif_sprite);
	quad.x = GetScreenPos().x + dif_sprite.x;
	quad.y = GetScreenPos().y + dif_sprite.y;
	SDL_SetTextureAlphaMod((SDL_Texture*)App->gui->GetAtlas(num_atlas), alpha);
	if (this->active)App->render->BlitInsideQuad((SDL_Texture*)App->gui->GetAtlas(num_atlas), sprite, quad);

	UI::PostUpdate();
	return true;
}

///////////// TEXT //////////////

TextUI::TextUI(Type type, UI* p, SDL_Rect r, std::string str, bool d, bool f, SDL_Rect d_area, bool console, SDL_Color coulor, bool title, Panel_Fade p_fade, int num_tooltip, bool tooltip_immediate)
	:UI(type, r, p, d, f, d_area, console, num_tooltip, tooltip_immediate)
{
	name.append("TextUI");
	stri = str.c_str();
	quad = r;
	color = coulor;
	title_default = title;
	fade_panel = p_fade;
	text = nullptr;

	if (fade_panel == Panel_Fade::no_one_fade) {
		alpha = 255;
	}
	if (fade_panel == Panel_Fade::panel_fade_in) {
		alpha = 0;
		fade_panel_timer.Start();
		fade_panel_time = 1;
	}
	SetString(stri);
}

bool TextUI::Update(float dt) {
	UI::Update(dt);

	return true;
}

bool TextUI::PostUpdate() {
	SDL_Rect rect = { 0,0,0,0 };
	iPoint dif_sprite = { 0,0 };

  
	SDL_QueryTexture(text, NULL, NULL, &rect.w, &rect.h);

	SDL_SetTextureAlphaMod(text, alpha);
	SDL_Rect sprite = UI::Check_Printable_Rect(rect, dif_sprite);
	if (this->active && this->GetConsole() == false)App->render->Blit(text, GetScreenToWorldPos().x + dif_sprite.x, GetScreenToWorldPos().y + dif_sprite.y, &sprite, 0.0F);
	else if (this->active) App->render->Blit(text, quad.x + dif_sprite.x, quad.y + dif_sprite.y, &sprite, 0.0F);
	UI::PostUpdate();

	return true;
}

bool TextUI::CleanUp()
{
	UI::CleanUp();
	App->tex->UnLoad(text);
	return true;
}

void TextUI::SetString(std::string new_string) 
{
	App->tex->UnLoad(text);
	stri = new_string;
	if (title_default == false)
		text = App->font->Print(stri.c_str(), color);
	else
		text = App->font->Print(stri.c_str(), color, App->font->default_title);
	if (text == nullptr) {
		LOG("TEXTURE NULLPTR");
	}
}

///////////// TEXT LIST //////////////

ListTextsUI::ListTextsUI(Type type, UI* p, SDL_Rect r, std::string str, bool d, bool f, SDL_Rect d_area, bool console, Panel_Fade p_fade) :UI(type, r, p, d, f, d_area, console) {
	name.append("ListTextsUI");
	stri.push_back(str);
	number_of_stri = stri.size();
	quad = r;
	PushBackTexture(str);

	fade_panel = p_fade;

	if (fade_panel == Panel_Fade::no_one_fade) {
		alpha = 255;
	}
	if (fade_panel == Panel_Fade::panel_fade_in) {
		alpha = 0;
		fade_panel_timer.Start();
		fade_panel_time = 1;
	}
}

bool ListTextsUI::Update(float dt) {
	UI::Update(dt);

	return true;
}

bool ListTextsUI::PostUpdate() 
{

	SDL_Rect rect = { 0,0,0,0 };
	iPoint dif_sprite = { 0,0 };

	for (int i = 0; i < number_of_stri; i++) 
	{
		dif_sprite = { 0,0 };

		std::list<std::string>::iterator it = stri.begin();
		std::list<SDL_Texture*>::iterator it_texture = text.begin();
		std::advance(it, i);
		std::advance(it_texture, i);

		if (it._Ptr->_Myval != stri.end()._Ptr->_Myval) 
		{

			SDL_QueryTexture(it_texture._Ptr->_Myval, NULL, NULL, &rect.w, &rect.h);

			SDL_SetTextureAlphaMod((SDL_Texture*)App->gui->GetAtlas(num_atlas), alpha);

			SDL_Rect sprite = UI::Check_Printable_Rect(rect, dif_sprite, { quad.x,quad.y + (quad.h * i),quad.w,quad.h });
			if (this->active && this->GetConsole() == false) App->render->Blit(it_texture._Ptr->_Myval, quad.x + dif_sprite.x, quad.y + dif_sprite.y + (i * quad.h), &sprite, 0.0F);
			else if (this->active) App->render->Blit(it_texture._Ptr->_Myval, quad.x + dif_sprite.x, quad.y + dif_sprite.y + (i * quad.h), &sprite, 0.0F);

			//App->tex->UnLoad(text);
		}
	}
	UI::PostUpdate();

	return true;
}

bool ListTextsUI::CleanUp()
{
	for (std::list<SDL_Texture*>::iterator it = text.begin(); it != text.end(); it++) {
		App->tex->UnLoad(it._Ptr->_Myval);
	}
	text.clear();
	stri.clear();
	return false;
}

void ListTextsUI::SetListOfStrings(std::string string, int position) 
{
	if (position > number_of_stri) {
		stri.push_back(string);
		PushBackTexture(string);
		number_of_stri++;
		SDL_Rect screen_rect = GetScreenRect();
		SDL_Rect parent_screen_rect = GetParentScreenRect();
		screen_rect.h += quad.h;
		quad.y = screen_rect.y = (parent_screen_rect.y + parent_screen_rect.h) - screen_rect.h;
		SetScreenRect(screen_rect);
		UpdateLocalRect();
	}
}

void ListTextsUI::PushBackTexture(std::string stri)
{
	text.push_back(App->font->Print(stri.c_str(), { 255,255,255,255 }));
}

///////////// BUTTON //////////////

ButtonUI::ButtonUI(Type type, UI * p, SDL_Rect r, SDL_Rect sprite, SDL_Rect spriten2, SDL_Rect spriten3, bool d, bool f, SDL_Rect d_area, int audio, Panel_Fade p_fade, int num_tooltip, bool tooltip_immediate)
	:UI(type, r, p, d, f, d_area, false, num_tooltip, tooltip_immediate) {
	name.append("ButtonUI");
	sprite1 = sprite;
	sprite2 = spriten2;
	sprite3 = spriten3;
	over = false;
	pushed = false;
	hover = false;
	quad = r;
	isLocked = false;
	front = true;
	click_sfx = App->gui->sfx_UI[audio];
	fade_panel = p_fade;

	if (fade_panel == Panel_Fade::no_one_fade) {
		alpha = 255;
	}
	if (fade_panel == Panel_Fade::panel_fade_in) {
		alpha = 0;
		fade_panel_timer.Start();
		fade_panel_time = 1;
	}
}

bool ButtonUI::Update(float dt) {
	UI::Update(dt);

	return true;
}

bool ButtonUI::PostUpdate() {
	SDL_Rect sprite;
	iPoint dif_sprite = { 0,0 };
	if (pushed == true || isLocked) {
		sprite = UI::Check_Printable_Rect(sprite2, dif_sprite);
	}
	else if (over == true) {
		sprite = UI::Check_Printable_Rect(sprite1, dif_sprite);
		if (hover == false) {
			hover = true;
			App->audio->PlayFx(1, App->gui->sfx_UI[(int)UI_Audio::HOVER]);
		}
	}
	else {
		sprite = UI::Check_Printable_Rect(sprite3, dif_sprite);
		hover = false;
	}
	//App->render->Blit((SDL_Texture*)App->gui->GetAtlas(), GetScreenToWorldPos().x + dif_sprite.x, GetScreenToWorldPos().y + dif_sprite.y, &sprite, 0.f);

	quad.x = GetScreenPos().x + dif_sprite.x;
	quad.y = GetScreenPos().y + dif_sprite.y;
	SDL_SetTextureAlphaMod((SDL_Texture*)App->gui->GetAtlas(num_atlas), alpha);
	if (this->active)App->render->BlitInsideQuad((SDL_Texture*)App->gui->GetAtlas(num_atlas), sprite, quad);

	UI::PostUpdate();
	return true;
}

bool ButtonUI::PreUpdate() {
	int x, y;
	App->input->GetMousePosition(x, y);
	bool pushing = false;
	if (front == true && ((x >= GetScreenPos().x && x <= GetScreenPos().x + GetScreenRect().w && y >= GetScreenPos().y && y <= GetScreenPos().y + GetScreenRect().h) || focus == true))
		over = true;
	else over = false;
	bool button = false;
	bool just_pushed = false;
	if (App->input->GetMouseButtonDown(1) == KEY_DOWN && over == true) {
		just_pushed = true;
	}
	if (App->input->GetMouseButtonDown(1) == KEY_REPEAT && pushed == true && over == true) {
		pushing = true;
	}
	if (App->input->GetMouseButtonDown(1) == KEY_UP && pushed == true && over == true)
		button = true;
	if (button == false)
		pushed = false;
	if (pushed && !App->gui->lockClick && !isLocked)
	{
		App->audio->PlayFx(1,click_sfx);
		//Button clicked
		if (listener)
		{
			listener->OnClick(this);
		}
		App->gui->lockClick = true;
	}
	if (just_pushed == true || pushing == true)
		pushed = true;
	UI::PreUpdate();

	return true;
}

///////////// TEXT INPUT //////////////

TextInputUI::TextInputUI(Type type, UI* p, SDL_Rect r, int re, int g, int b, int a, std::string str, bool d, bool f, SDL_Rect d_area, Panel_Fade p_fade) :UI(type, r, p, d, f, d_area, true) {
	name.append("TextInputUI");
	sprite1 = { 0,0,0,0 };
	quad = r;
	label = str;
	text_input = false;
	position = 0;
	square = true;
	red = re;
	green = g;
	blue = b;
	alpha = a;
	fade_panel = p_fade;
	text = nullptr;

	if (fade_panel == Panel_Fade::no_one_fade) {
		alpha = 255;
	}
	if (fade_panel == Panel_Fade::panel_fade_in) {
		alpha = 0;
		fade_panel_timer.Start();
		fade_panel_time = 1;
	}
	UpdateLabel();
}

bool TextInputUI::Update(float dt) {
	UI::Update(dt);

	return true;
}

bool TextInputUI::PreUpdate() {
	int x, y;
	App->input->GetMousePosition(x, y);
	if ((x >= GetScreenPos().x && x <= GetScreenPos().x + GetScreenRect().w && y >= GetScreenPos().y && y <= GetScreenPos().y + GetScreenRect().h)) {
		if (App->input->GetMouseButtonDown(1) == KEY_DOWN) {
			App->gui->DeleteFocus();
			focus = true;
		}
	}
	if (focus == true && text_input == false) {
		SDL_StartTextInput();
		text_input = true;
	}
	else if (focus == false && text_input == true) {
		SDL_StopTextInput();
		text_input = false;
	}
	if (focus == true) 
	{
		if (App->input->special_keys == specialkeys::Backspace) 
		{
			if (position > 0)
			{
				label = label.erase(position - 1, 1);
				position--;
				UpdateLabel();
			}
		}
		else if (App->input->special_keys == specialkeys::Left) {
			if (position > 0)
				position--;
		}
		else if (App->input->special_keys == specialkeys::Right) {
			if (position < label.size())
				position++;
		}
		else if (App->input->special_keys == specialkeys::Supr) {
			if (position > 0) 
			{
				label = label.erase(position - 1);
				position--;
				UpdateLabel();
			}
		}
		else if (App->input->special_keys == specialkeys::Home) {
			position = 0;
		}
		else if (App->input->special_keys == specialkeys::End) {
			position = label.size();
		}

	}
	UI::PreUpdate();
	return true;
}

bool TextInputUI::PostUpdate() {
	iPoint dif_sprite = { 0,0 };
	SDL_SetTextureAlphaMod((SDL_Texture*)App->gui->GetAtlas(num_atlas), alpha);
	if (this->active) {
		App->render->DrawQuad(quad, red, green, blue, alpha, true, false);
	}

	SDL_Rect rect = { 0,0,0,0 };
	if (strcmp(label.c_str(), "")) {
		SDL_QueryTexture(text, NULL, NULL, &rect.w, &rect.h);
		SDL_Rect sprite = UI::Check_Printable_Rect(rect, dif_sprite);
		if (this->active) App->render->Blit(text, quad.x + dif_sprite.x, quad.y + dif_sprite.y, &sprite, 0.0F);
	}

	if (focus == true) 
	{
		std::string label_from_position = label;
		label_from_position.erase(label_from_position.begin() + position, label_from_position.end());
		int w, h;
		App->font->CalcSize(label_from_position.c_str(), w, h);
		App->render->DrawLine(quad.x + w, quad.y, quad.x + w, quad.y + quad.h, 0, 255, 255,alpha, false);
	}
	UI::PostUpdate();



	return true;
}

bool TextInputUI::CleanUp()
{
	App->tex->UnLoad(text);
	return false;
}

//BUG: when moving with left or right keys and typing
void TextInputUI::ChangeLabel(std::string text) 
{
	std::string label_part_2, label_part_1;
	label_part_1 = label_part_2 = label;
	for (unsigned i = position; i < label.size(); i++) 
	{
		label_part_1.erase(position);
	}
	if (strcmp(label.c_str(), label_part_1.c_str())) 
	{
		for (int i = 0; i < position; i++) 
		{
			label_part_2 = label_part_2.erase(0);
		}
	}
	else {
		label_part_2 = "";
	}
	label_part_1 += text.c_str();
	label_part_1 += label_part_2;
	label = label_part_1;
	position++;
	UpdateLabel();
}

void TextInputUI::SetLabel(std::string text) 
{
	if (App->input->GetKey(SDL_SCANCODE_GRAVE) != KEY_DOWN) 
	{
		label = text.c_str();
		position += label.size() - 1;
		UpdateLabel();
	}
}

void TextInputUI::SetPositionToZero() {
	position = 0;
}

void TextInputUI::UpdateLabel()
{
	App->tex->UnLoad(text);
	text = App->font->Print(label.c_str(), { 255,255,255,255 });
}

void j1Gui::DoWinOrLoseWindow(int type, bool win) {
	SDL_Rect sec_viking = { 0, 0,807, 345 };
	SDL_Rect sec_greek = { 0, 345,807, 345 };

	SDL_Rect sec_win = { 807, 0,807, 345 };
	SDL_Rect sec_lose = { 807, 345,807, 345 };

	SDL_Rect sec_tutorial = { 0, 690,807, 345 };
	SDL_Rect sec_completed = { 807, 690,807, 345 };

	if (App->scene->hud != nullptr) {
		if (App->scene->hud->start_timer == false) {
			App->scene->hud->timer_win_lose.Start();
			animation_win_lose_timer.Start();
		}

		App->scene->hud->start_timer = true;
	}

	if (animation_win_lose_timer.ReadSec() <= 2) {
		global_pos = DoTransitionWinLose(230, 100, winlose_tex, animation_win_lose_timer);
	}

	else if (animation_win_lose_timer.ReadSec() >= 2) {
		global_pos.y = 100;
	}

	if (type == 1) {
		if (win == true) {
			App->render->Blit(winlose_tex, global_pos.x, global_pos.y, &sec_viking, NULL, 0.0F);
			App->render->Blit(winlose_tex, global_pos.x, global_pos.y, &sec_win, NULL, 0.0F);
			if (Mix_Playing(3) == 0)
			{
				App->audio->PlayFx(3, App->scene->WinViking_sound);
			}
		}
		else {
			App->render->Blit(winlose_tex, global_pos.x, global_pos.y, &sec_viking, NULL, 0.0F);
			App->render->Blit(winlose_tex, global_pos.x, global_pos.y, &sec_lose, NULL, 0.0F);
			if (Mix_Playing(3) == 0) {
				App->audio->PlayFx(3, App->scene->Lose_sound);
			}
		}
	}

	if (type == 2) {
		if (win == true) {
			App->render->Blit(winlose_tex, global_pos.x, global_pos.y, &sec_greek, NULL, 0.0F);
			App->render->Blit(winlose_tex, global_pos.x, global_pos.y, &sec_win, NULL, 0.0F);
			if (Mix_Playing(3) == 0) {
				App->audio->PlayFx(3, App->scene->WinGreek_sound);
			}
		}
		else {
			App->render->Blit(winlose_tex, global_pos.x, global_pos.y, &sec_greek, NULL, 0.0F);
			App->render->Blit(winlose_tex, global_pos.x, global_pos.y, &sec_lose, NULL, 0.0F);
			if (Mix_Playing(3) == 0) {
				App->audio->PlayFx(3, App->scene->Lose_sound);
			}
		}
	}

	if (type == 3) {
		App->render->Blit(winlose_tex, global_pos.x, global_pos.y, &sec_tutorial, NULL, 0.0F);
		App->render->Blit(winlose_tex, global_pos.x, global_pos.y, &sec_completed, NULL, 0.0F);
		if (Mix_Playing(3) == 0)
		{
			App->audio->PlayFx(3, App->scene->WinViking_sound);
		}
	}

	if (App->scene->hud != nullptr) {
		if (App->scene->hud->timer_win_lose.ReadSec() >= 5) {

			App->scene->BackToTitleMenu();
		}
	}
}

fPoint j1Gui::DoTransitionWinLose(int pos_x, int pos_y, SDL_Texture* tex, j1Timer time) {
	fPoint position_global;
	position_global.x = pos_x;

	if (first_time_timer_win == false) {
		animation_win_lose_timer.Start();
		first_time_timer_win = true;
	}

	float percentatge = time.ReadSec() * 0.5f;
	position_global.y = LerpValue(percentatge, 200, 100);

	SDL_SetTextureAlphaMod(tex, 255 * percentatge);

	return position_global;
}

float j1Gui::LerpValue(float percent, float start, float end)
{
	return start + percent * (end - start);
}
