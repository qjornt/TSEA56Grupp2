//
//  Manual.h
//  GUI
//
//  Created by Andreas Brorsson on 2015-04-21.
//  Copyright (c) 2015 Andreas Brorsson. All rights reserved.
//

#ifndef __GUI__Manual__
#define __GUI__Manual__

#include <stdio.h>
#ifdef __APPLE__
#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <SDL2_ttf/SDL_ttf.h>
#include <SDL2_mixer/SDL_mixer.h>
#endif

#ifdef __linux__
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#endif


#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
//#include "../../KONSTRUKTION/KOD/Datorvisualisering_/SDL2_BT_v0.1/SDL2_BT_v0.1/Bluetooth.h"


#include <string>
#include <vector>
#include <stdio.h>
#include "State.h"
#include "Objects.h"


using namespace std;



class Manual: public State
{

public:
    
    Manual(SetupSDL* sdl_lib, void*);
    ~Manual();
    
    virtual void      event(string& statestring,bool& running) override;
    virtual void      update(string& statestring,bool& running)  override; //i cc - object_vector -> update()
    virtual void      render()  override; //i cc - object_vector -> render()
    virtual void      run(string& statestring) override;
    void Set_Speed();
    void init_gfx_win();
    void init_gfx_mac();
	void init_text();
	void render_text();
	void update_text(int, int, int, int, int, int);
	void Check_Mode(string&, bool&);
    
private:

    SDL_Renderer* renderer_;
    SDL_Event* mainevent_;

	HANDLE hComm;

    Unmoveable_Object* Bakgrund;
    Moveable_Object* Pil1;
    Moveable_Object* Pil2;
    
    //Inerna IN --> UT
    int8_t Speed_Horizont;
    int8_t Speed_Vertical;
    uint8_t Speed;
    uint8_t Speed_right;
    uint8_t Speed_left;
	uint8_t Dir_left;
	uint8_t Dir_right;
	uint8_t Klo;
	int8_t arrSensor[7];

    int8_t agg;
    
	Text* Text_Angle_;
	Text* Text_Offset_;
	Text* Text_Reflex_;
	Text* Text_Front;
	Text* Text_Clear_right_;
	Text* Text_Clear_left_;

    //Externa IN <-- UT
    int8_t angle_;
    int8_t offset_;
    int8_t reflex_;
    int8_t space_right_;
    int8_t space_left_;
    int8_t front_ = 20;

	const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

    //Plats för texturer
//    //Mac
    //string Font_ = "/Users/Andreas/Library/Fonts/DS-DIGI.TTF";
    //string Pil_plats = "/Users/Andreas/Skola/KP2/GUI/Bilder/Pil.png";
    //string BG_plats = "/Users/Andreas/Skola/KP2/GUI/Bilder/Manual.png";
    
    //Windows
	string Font_ = "C:/Users/Måns/Documents/GitHub/TSEA56Grupp2/GUI/Bilder/ds_digital/DS-DIGI.TTF";
	string Pil_plats = "C:/Users/Måns/Documents/GitHub/TSEA56Grupp2/GUI/Bilder/Pil.png";
	string BG_plats = "C:/Users/Måns/Documents/GitHub/TSEA56Grupp2/GUI/Bilder/Manual.png";
    
    
};



#endif /* defined(__GUI__Manual__) */
