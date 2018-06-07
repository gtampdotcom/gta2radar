//GTA2 Radar by Sektor GTAMP.com gtamult@gmail.com 2017
//gta2minimap generator by T.M. (closed source)
//Do whatever you want with this "code"
//Don't let your dreams be dreams
//Nothing is impossible
//Just do it

#define _GUI
//#define _parkedcars

#include <SDL.h>

#ifdef _GUI
#include <SDL_image.h>
#endif

#include <stdio.h>
#include <iostream>
#include <windows.h>
#include <psapi.h> // For access to GetModuleFileNameEx
#include <tlhelp32.h>
#include <array>
#include <algorithm>
#include <sstream>

using namespace std;

const int gta2base_addr = 0x3F0000;
//#define GTA2_ADDR_PLAYER_IN_VEHICLE (char*)0x5e20bc

bool bRadar;
bool thingschanged;
bool renderWindow;
bool sektor;

int maprendercount = 0;
int prevFrameCount = 0;
int p1angle;
int p1_addr;
int prevTicks;
int objectlimit = 1;
int pedxyz [200][3];
int oldpedxyz [200][3];
int carxyz [200][3];
int medialoaded = 0;
int oldWindowWidth;
int oldWindowHeight;
int object_number;
int fraglimit;
//int p1frags,p2frags,p3frags,p4frags,p5frags,p6frags;
//int p1kills,p2kills,p3kills,p4kills,p5kills,p6kills;
int ped;
int p1x,p1y,p1z;
int pedx,pedy;
int playernum;

int object_ptr = gta2base_addr+0x1E8750;
int savedgame_first_object_ptr = gta2base_addr+0x275784;
int first_object_ptr = gta2base_addr+0x275788; // = ptr + c = ptr + 2c = first object
int multiplayer_addr = 0x673E2C;
int vehicle_addr;
int p1car_ptr = gta2base_addr+0x1F4CA0;
int p1car_id = gta2base_addr+0x1EE604;
int object01_ptr = gta2base_addr+0x1E86D0; //+14
int car01struct_ptr = gta2base_addr+0x1F4CA0; //+4 car1 struct ptr
int car01struct_offset;
int car01struct;
int car01sprite;
int car01_x;
int car01_y;
int objects_ptr = gta2base_addr+0x1E86C0; //+0x50 for x position
int objects;
int objects_x;
int objects_y;
int object01;
int object01_x;
int object01_y;
int car_id_number;
LPVOID pinfo_ptr = (void*)0x5EB4FC;
int ped_addr;

//Screen dimension
int SCREEN_WIDTH = 768;
int SCREEN_HEIGHT = SCREEN_WIDTH;

float oldscalex;
float oldscaley;
float zoom = 1;
float savex;
float savey;
float fParkedX,fParkedY;
float fPlayerX,fPlayerY,fPlayerZ;

string prevcurl;
string gta2modulename;
string strgmp;
string oldgmpfile="furp";

TCHAR gta2exe[MAX_PATH];

//wchar_t names[6][32];
//wstring names[6][32];
wchar_t p1name[32],p2name[32],p3name[32],p4name[32],p5name[32],p6name[32];

char gmpfile[MAX_PATH];
char styfile[MAX_PATH];
char scrfile[MAX_PATH];
char mmpfile[MAX_PATH];
char imagefile[2];

//GTA2 memory addr
//do_show_ids
//byte at 5EADA1
LPVOID framecount_ptr = (void*)0x5E8108;
LPVOID p1name_addr = (void*)0x5EC524;
LPVOID p2name_addr = (void*)0x5EC544;
LPVOID p3name_addr = (void*)0x5EC564;
LPVOID p4name_addr = (void*)0x5EC584;
LPVOID p5name_addr = (void*)0x5EC5A4;
LPVOID p6name_addr = (void*)0x5EC5C4;
LPVOID gmp_addr = (void*)0x5EC075;
LPVOID sty_addr = (void*)0x5EC175;
LPVOID scr_addr = (void*)0x5EC275;
LPVOID mmp_addr = (void*)0x673E30;
LPVOID ped_ptr = (void*)0x5D85C0;

#ifdef _GUI

//Texture wrapper class
class LTexture
{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		//Loads image at specified path
		bool loadFromFile( string path );

		//Deallocates texture
		void free();

		//Set color modulation
		void setColor( Uint8 red, Uint8 green, Uint8 blue );

		//Set blending
		void setBlendMode( SDL_BlendMode blending );

		//Set alpha modulation
		void setAlpha( Uint8 alpha );
		
		//Renders texture at given point
		void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

		//Gets image dimensions
		int getWidth();
		int getHeight();

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;

		//Image dimensions
		int mWidth;
		int mHeight;
};
#endif

#ifdef _GUI
//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

void loadIcons();

//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Scene texture
LTexture gArrowTexture[6];
LTexture gMapTexture;
LTexture gSaveTexture;
LTexture gCopTexture;
LTexture gMuggerTexture;
LTexture gCarThiefTexture;
LTexture gCarTexture;
//LTexture gTokenTexture;
LTexture gObjectTexture[300];

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile( string path )
{
	//printf("loadFromFile: %s\n",path.c_str());
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		//printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else
	{
		//Color key image
		SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0xFF, 0, 0xFF ) );
		
		//Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
			
			if (mWidth<=512)
			{
				mWidth=mWidth*zoom;
			}
			
			if (mHeight<=512)
				mHeight=mHeight*zoom;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

void LTexture::free()
{
	//Free texture if it exists
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{
	//Modulate texture rgb
	SDL_SetTextureColorMod( mTexture, red, green, blue );
}

void LTexture::setBlendMode( SDL_BlendMode blending )
{
	//Set blending function
	SDL_SetTextureBlendMode( mTexture, blending );
}
		
void LTexture::setAlpha( Uint8 alpha )
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Create window
		//gWindow = SDL_CreateWindow( "GTA2 Radar v0.1", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		//gWindow = SDL_CreateWindow( "GTA2 Radar v0.1", 0, 20, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE );
		gWindow = SDL_CreateWindow( "GTA2 Radar v0.2", 0, 20, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			SDL_Surface* icoSurface = IMG_Load("images\\gta2radar.png");
			SDL_SetWindowIcon(gWindow,icoSurface);
			
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;
	
	if(strcmp(scrfile,"") != 0)
	{	
		//Load map
		string strscr = scrfile;
		string strsty = styfile;
		string strgmp = gmpfile;
		string strgta2 = gta2exe;
		string strjpg = gmpfile;
		//remove extension from gmp file for jpg filename
		if(strjpg.size()>4)
		{
			unsigned sz = strjpg.size()-4;
		  	strjpg.resize (sz);
		}
		//remove \gta2.exe from gta2 path since we just want the folder
		strgta2=strgta2.substr(0, strgta2.find_last_of("\\"));	
		
		strgta2=strgta2+"\\data\\";
		strjpg=strjpg+".jpg";
	
		//get the path to gta2radar.exe
		char buffer[MAX_PATH];
		GetModuleFileName(NULL,buffer,sizeof(buffer));
		//cut off gta2radar.exe and keep the rest of the folder
		string radarpath = buffer;
		radarpath=radarpath.substr(0, radarpath.find_last_of("\\"));	
		string width="1024"; //  gta2minimap.exe only supports 256, 512, 1024 and 2048
		
		if (strcmp(strgmp.c_str(),oldgmpfile.c_str()) != 0)
		{
			if( !gMapTexture.loadFromFile("maps\\" + strjpg))
			{
				oldgmpfile="furp";
			}
			else
			{
				oldgmpfile=strgmp;
			}
					
			if(gMapTexture.getWidth()!=atoi(width.c_str())) // // gta2minimap.exe only supports 256, 512, 1024 and 2048
			{
				oldgmpfile="furp";
			}
	 	
			if (strcmp(strgmp.c_str(),oldgmpfile.c_str()) != 0)
			{	
				string minimap;
				minimap=radarpath + "\\minimap\\gta2minimap.exe" + " -saveminimap -sty \"" + strgta2 + strsty + "\" -gmp \"" + strgta2 + strgmp + "\" -out \"" + radarpath + "\\maps\\" + strjpg + "\" -size " + width + " -bluewater 1";
				printf("Exec: %s\n",minimap.c_str());
				system(minimap.c_str());
				gMapTexture.loadFromFile("maps\\" + strjpg);
				oldgmpfile=strgmp;	
			}
		}
	}
	
	return success;
}

void close()
{
	//Free loaded images
	gMapTexture.free();
	gArrowTexture[0].free();
	gSaveTexture.free();
	gCopTexture.free();
	gMuggerTexture.free();
	gCarThiefTexture.free();
	gCarTexture.free();
	//gTokenTexture.free();
	gObjectTexture[0].free();

	//Destroy window	
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;
	
	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}
#endif

int main( int argc, char* args[] )
{
	int angle;
	int pedcar;
	int playercount;
	
	#ifdef _GUI
	int mapWidth;
	int mapHeight;
	
	//Start up SDL and create window
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{	
	#endif
	
		//Main loop flag
		bool quit = false;
		
		#ifdef _GUI	
		//Event handler
		SDL_Event e;


		//Angle of rotation
		double degrees = 0.0;
		
		//Flip type
		SDL_RendererFlip flipType = SDL_FLIP_NONE;
		#endif		
		
		printf("GTA2 Radar v0.2 by Sektor GTAMP.com\n");
		printf("-----------------------------------\n");
		
		#ifdef _GUI
		loadIcons();
		#endif
		
		//While application is running
	
		
		while( !quit )
		{

			#ifdef _GUI
			//Handle events on queue
			while( SDL_PollEvent( &e ) != 0 )
			{
				//User requests quit
				if( e.type == SDL_QUIT )
				{
					quit = true;
				}
				else if( e.type == SDL_KEYDOWN )
				{
					switch( e.key.keysym.sym )
					{
						case SDLK_MINUS:
						if(zoom>=0.5)
						{
							zoom-=0.25;
							loadIcons();
						}
						break;
						
						case SDLK_EQUALS:
						zoom+=0.25;
						loadIcons();
						break;
					
					}
				}
			}
			#endif
					
			HANDLE hProcessSnap;	// will store a snapshot of all processes
			HANDLE hProcess = NULL;	// we will use this one for the GTA2 process
			PROCESSENTRY32 pe32;	// stores basic info of a process, using this one to read the ProcessID from
		
			hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);	// make process snapshot
		
			pe32.dwSize = sizeof(PROCESSENTRY32);		// correct size
		
			Process32First(hProcessSnap, &pe32);	// read info about the first process into pe32
				
			do	// loop to find the process
			{
				if (strcmp(pe32.szExeFile, "gta2.exe") == 0)	// if process was found
				{
					hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);	// open it, assigning to the hProcess handle
					
					//GetModuleBaseName(hProcess, NULL, gta2exe, MAX_PATH);
					//cout << "Module:" << gta2exe ;
				    
					GetModuleFileNameEx(hProcess, NULL, gta2exe, MAX_PATH);
					
          			break;	// break the loop
				}
			} while (Process32Next(hProcessSnap, &pe32));	// loop continued until Process32Next deliver NULL or its interrupted with the "break" above
		
			CloseHandle(hProcessSnap);	// close the handle
			
			if (hProcess == NULL)
			{
				#ifdef _GUI
		    	medialoaded=0;
				SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0x00 );
				SDL_RenderClear( gRenderer );
				
				p1x=SCREEN_WIDTH/2;
				p1y=SCREEN_HEIGHT/2;
				
		    	SDL_RenderPresent( gRenderer );
		    	oldgmpfile="furp";
		    	SDL_RenderClear( gRenderer );
		    	#endif
		    	
			}
			else
			{
				int pinfo_addr=0;
				int p1_ptr;
				//int p1_addr;
				int arrayofplayerptrs;
				int playercount_addr;
				int player_info;
				ReadProcessMemory(hProcess, pinfo_ptr, &pinfo_addr, 4, NULL);
				arrayofplayerptrs=pinfo_addr+0x4;
				ReadProcessMemory(hProcess, (LPVOID)arrayofplayerptrs, &player_info, 4, NULL);
				p1_ptr=player_info+0xC4;
				ReadProcessMemory(hProcess, (LPVOID)p1_ptr, &p1_addr, 4, NULL);
				
				playercount_addr=pinfo_addr+0x23;
				ReadProcessMemory(hProcess, (LPVOID)playercount_addr, &playercount, 1, NULL);
				
				ReadProcessMemory(hProcess, gmp_addr, &gmpfile, 32, NULL);
				ReadProcessMemory(hProcess, sty_addr, &styfile, 32, NULL);
				ReadProcessMemory(hProcess, scr_addr, &scrfile, 32, NULL);
				ReadProcessMemory(hProcess, mmp_addr, &mmpfile, 32, NULL);
				
				ReadProcessMemory(hProcess, p1name_addr, &p1name, 32, NULL);
				ReadProcessMemory(hProcess, p2name_addr, &p2name, 32, NULL);
				ReadProcessMemory(hProcess, p3name_addr, &p3name, 32, NULL);
				ReadProcessMemory(hProcess, p4name_addr, &p4name, 32, NULL);
				ReadProcessMemory(hProcess, p5name_addr, &p5name, 32, NULL);
				ReadProcessMemory(hProcess, p6name_addr, &p6name, 32, NULL);
				
				int framecount,framecount_addr;
				ReadProcessMemory(hProcess, framecount_ptr, &framecount_addr, 4, NULL);
				ReadProcessMemory(hProcess, (LPVOID)framecount_addr, &framecount, 4, NULL);
				//cout << "framecount: " << framecount << "\n";
				
				#ifdef _GUI
				
				//change icon if player name contains sektor
				char str[32];
			    wcstombs(str, p1name, 31);
				string data = str;
				transform(data.begin(), data.end(),
				data.begin(), ::tolower);
				size_t found;
				found = data.find("sektor");
				if (found!=string::npos)
				{
					if(!sektor)
					{
						sektor=true;
						gArrowTexture[0].loadFromFile( "images\\sektor.png" );
					}
				}
				
				if(playercount>6 or playercount<1)
				{
					playercount=1;
				}
					
				//disable radar unless p1name or mmp filename contains the word radar
				if(playercount>=2)
				{	
					string mmpdata = mmpfile;
  					if (mmpdata.find("radar")!=string::npos || data.find("radar")!=string::npos) 
  					{
    					bRadar=true;
    				}
    				else
    				{
    					bRadar=false;	
    				}
    			}
    			else
    			{		
    				bRadar=true;
    			}
			
				
				if(bRadar)
				{	
					//cout << "Radar enabled\n";
					loadMedia();
					
					if (thingschanged)
					{
						mapWidth=gMapTexture.getWidth();
						mapHeight=gMapTexture.getWidth();
					
						//Clear screen
						//SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0x00 );
						//SDL_RenderClear( gRenderer );
					
						//Render map
						//if (maprendercount<2)
						{
							//++maprendercount;
							gMapTexture.render(0,0);
						}
						
					}
						int first_object_ptr2 = 0;
						int first_object_addr = 0;
						int first_object_value = 0;
						int savedgame_first_object_ptr2 = 0;
						int objectspace=100000;
						unsigned char tokenmemspace[objectspace];
			
						// first object location is different in saved games and new games
						// there must be a better way to get the address for the first object but using different ptrs for new or save seems to work
						ReadProcessMemory(hProcess, (LPVOID)savedgame_first_object_ptr, &savedgame_first_object_ptr2, 4, NULL);
						savedgame_first_object_ptr2+=52;
						ReadProcessMemory(hProcess, (LPVOID)savedgame_first_object_ptr2, &first_object_ptr2, 4, NULL);
					
						
						if(first_object_ptr2>0) // true if saved game
						{
							first_object_ptr2+=44;
						}
						else
						{	
							// it's probably a new game
							ReadProcessMemory(hProcess, (LPVOID)first_object_ptr, &first_object_ptr2, 4, NULL);
							first_object_ptr2+=0xC;
						}
						
						ReadProcessMemory(hProcess, (LPVOID)first_object_ptr2, &first_object_addr, 4, NULL);
						
						//cout << "first object addr: " << first_object_addr << "\n";
						
						
						if(first_object_addr>0) //only read object memory if the game has started and isn't just on the menu
						{
							first_object_addr+=24;	
							ReadProcessMemory(hProcess, (LPVOID)first_object_addr, &tokenmemspace[0], objectspace, NULL);
							int token_addr = 0;
							int tokenx = 0;
							int tokeny = 0;
							int object_z = 0;
							int tokenx_addr = 0;
							int tokeny_addr = 0;
							int object_z_addr = 0;
							
							float fTokenX;
							float fTokenY;
							
							int first_token_addr;
							
							int realfirstobject_addr=first_object_addr;
							
							int invulcount = 0;
							int ddcount = 0;
							int frcount = 0;
							int inviscount = 0;
							int tokencount = 0;
							int kfcount = 0;
							
							for(int i = 0; i <objectspace; i=i+44)
							{
								int object_num = 0xFFFF; 
								int object_addr;
								memcpy ( &object_num, &tokenmemspace[i], 2 ); //copy 2 bytes to get object number	
			
								//ignore object 122 (traffic lights)
								
								// Find invul by searching EA 00 00 00 00 00 00 00 01
								// You can find gta2 tokens/icons by searching for hex string 0A0100000000000002
								// 0A 01 00 00 00 00 00 00 02
								
								switch(object_num)
								{
									case 234:
									++invulcount;
									break;
								
									case 235:
									++ddcount;
									break;
									
									case 236:
									++frcount;
									break;
									
									case 239:
									++inviscount;
									break;
									
									case 266:
									++tokencount;
									break;
									
									case 286:
									++kfcount;
									break;
								}

								{
									//printf("mem: %d\n",object_num);
									realfirstobject_addr=first_object_addr+i-20;
									ReadProcessMemory(hProcess, (LPVOID)realfirstobject_addr, &token_addr, 4, NULL);
									tokenx_addr=token_addr+20;
									tokeny_addr=token_addr+24;
									object_z_addr=token_addr+28;
									ReadProcessMemory(hProcess, (LPVOID)tokenx_addr, &tokenx, 4, NULL);
									ReadProcessMemory(hProcess, (LPVOID)tokeny_addr, &tokeny, 4, NULL);			
									ReadProcessMemory(hProcess, (LPVOID)object_z_addr, &object_z, 4, NULL);			
									fTokenX =(float) tokenx/16384;
									fTokenY =(float) tokeny/16384;	
									
									//printf("object %d %.02f %.02f %d\n",object_num,fTokenX,fTokenY,realfirstobject_addr);	
									//if(!gObjectTexture[object_num].getHeight())
									//{
									//	printf("object %d %.02f %.02f %d\n",object_num,fTokenX,fTokenY,tokenx_addr);	
								//		gObjectTexture[object_num].loadFromFile( "images\\arrow.png" );
								//	}
										
										
									int object_angle;
									ReadProcessMemory(hProcess, (LPVOID)token_addr, &object_angle, 4, NULL);
									//if(p1angle>0)
									{
										//printf("object %d %d %d %d\n",token_addr,object_angle,p1angle,object_z);	
										//i=100000;
									}
										
									gObjectTexture[object_num].render( fTokenX*(mapWidth/256)-gObjectTexture[object_num].getWidth()/2, fTokenY*(mapHeight/256)-gObjectTexture[object_num].getHeight()/2, NULL, degrees, NULL, flipType );
									//gObjectTexture[56].render( fTokenX*(mapWidth/256)-gObjectTexture[56].getWidth()/2, fTokenY*(mapHeight/256)-gObjectTexture[56].getHeight()/2, NULL, degrees, NULL, flipType );	
								}
							}
	
						//cout << "tokens " << tokencount << " invul " << invulcount << "\n";
						stringstream ss_title;
						ss_title << "GTA2 Radar v0.2 " << "tokens " << tokencount << " invul " << invulcount << " dd " << ddcount << " fr " << frcount << " invis " << inviscount << " kf " << kfcount;
						string s_title = ss_title.str();
						const char* c_title = s_title.c_str();
						SDL_SetWindowTitle(gWindow, c_title);
						}
					
					
					
					//save points/church/jesus saves location	
					if(strcmp(scrfile,"bil.scr") == 0)
					{	
			          savex = 44.5;
			          savey = 100.5;
			          gSaveTexture.render( savex*(mapWidth/256)-gSaveTexture.getWidth()/2, savey*(mapHeight/256)-gSaveTexture.getHeight()/2, NULL, degrees, NULL, flipType );
			        }
			
			        if(strcmp(scrfile,"wil.scr") == 0)
					{
			          savex = 159;
			          savey = 137;
			          gSaveTexture.render( savex*(mapWidth/256)-gSaveTexture.getWidth()/2, savey*(mapHeight/256)-gSaveTexture.getHeight()/2, NULL, degrees, NULL, flipType );
			        }
			        
			        if(strcmp(scrfile,"ste.scr") == 0)
					{
			          savex = 113;
			          savey = 123;
			          gSaveTexture.render( savex*(mapWidth/256)-gSaveTexture.getWidth()/2, savey*(mapHeight/256)-gSaveTexture.getHeight()/2, NULL, degrees, NULL, flipType );
			        }	
					#endif
		
					/*
					int i;
					int j;
					
					i=0;
					j=0;
					
					car_id_number=0;
					
					for(int i = 0; i <200; i++) {
						
					int p1car_meh=0;
					int p1car_x=0;
					int p1car_y=0;
					int p1car_struct=0;
					
					ReadProcessMemory(hProcess, (LPVOID)p1car_id,&p1car_struct, 4, NULL);	
					p1car_struct+=0x4;
					ReadProcessMemory(hProcess, (LPVOID)p1car_struct,&p1car_meh, 4, NULL);	
					
					car_id_number+=0xBC;
					p1car_meh+=0x7c4-car_id_number;
					//p1car_meh+=0x38C4-car_id_number;
					//p1car_meh-=0x3100-car_id_number;
					//p1car_meh=p1car_meh-0xBC;
					
					ReadProcessMemory(hProcess, (LPVOID)p1car_meh,&p1car_struct, 4, NULL);	
					printf("car ID: %d addy: %d\n",p1car_struct,p1car_meh);
					
					p1car_meh-=20;
					ReadProcessMemory(hProcess, (LPVOID)p1car_meh,&p1car_struct, 4, NULL);	
					p1car_struct+=48;
					ReadProcessMemory(hProcess, (LPVOID)p1car_struct,&p1car_x, 4, NULL);
					
					//printf("car x: %d\n",p1car_x/16384);
					
					p1car_struct+=4;
					ReadProcessMemory(hProcess, (LPVOID)p1car_struct,&p1car_y, 4, NULL);
					//printf("car y: %d\n",p1car_y/16384);
					
						if(p1car_x!=0) {
						
							p1car_x/=16384;
							p1car_y/=16384;
							//carxyz[i][0]=p1car_x;
							//carxyz[i][1]=p1car_y;
								
							gCarTexture.render( p1car_x*(SCREEN_WIDTH/256), p1car_y*(SCREEN_WIDTH/256), NULL, degrees, NULL, flipType );
						}	
					}
					*/
					
					// dummy peds and players
					char c1x[16]="0";
					char c1y[16]="0";
					char c1a[16]="0";
					char c2x[16]="0";
					char c2y[16]="0";				
					char c2a[16]="0";				
					
					int ped_sprite_ptr;
					int ped_sprite_addr;
					int ped_angle_addr;
					int p1struct=0;
					int angle[100];
					int oldangle[100];
					object_number=0;
					
					if(playercount>6 or playercount<1)
					{
						playercount=1;
					}
					
					for(int i = 0; i <playercount; i++)
					{	
						p1struct=p1_addr;
						int health=666;
		      			int health_addr=0;
						health_addr=p1struct+534;
						ReadProcessMemory(hProcess, (LPVOID)health_addr,&health, 2, NULL);
						//WriteProcessMemory(hProcess, (LPVOID)health_addr,&health, 2, NULL);
						//printf("health: %d\n",health);	
		      			//printf("p1struct: %d\n",p1struct);
		      			p1struct+=object_number;
		      			//int invis_addr=0;
						//invis_addr=p1struct+540;
						//int invis=33554433;
						//WriteProcessMemory(hProcess, (LPVOID)invis_addr, &invis, 4, NULL);
		      			
		      			
		      			int pedstruct;
		      			pedstruct=p1struct;
		      			int ped_occupation_addr=0;
		      			int ped_remap_addr=0;
		      			ped_occupation_addr=p1struct+0x240;
		      			ped_remap_addr=p1struct+0x244;
		      			ped_sprite_ptr=p1struct+360;
						
						int pedremap_addr=pedstruct+0x244;
						int pedremap=0;  
						ReadProcessMemory(hProcess, (LPVOID)pedremap_addr,&pedremap, 2, NULL);
						
						
						
						//p1struct gets fucted
						p1struct+=0x1AC;
		      			ReadProcessMemory(hProcess, (LPVOID)p1struct,&pedx, 4, NULL);
		      			p1struct+=0x4;
		      			ReadProcessMemory(hProcess, (LPVOID)p1struct,&pedy, 4, NULL);
		      			
		      			fPlayerX = (float) pedx/16384;
		      			fPlayerY = (float) pedy/16384;
		      			//printf("x %.02f y %.02f\n",fPlayerX,fPlayerY);
		      			
		      			int occupation=0;
		      			int remap=0;	      			
						
		      			ReadProcessMemory(hProcess, (LPVOID)ped_occupation_addr,&occupation, 2, NULL);
		      			ReadProcessMemory(hProcess, (LPVOID)ped_remap_addr,&remap, 2, NULL);
		      			pedx/=16384;
						pedy/=16384;
						
						ReadProcessMemory(hProcess, (LPVOID)ped_sprite_ptr,&ped_sprite_addr, 4, NULL);
						ped_sprite_addr+=128;
	  					ReadProcessMemory(hProcess, (LPVOID)ped_sprite_addr,&ped_angle_addr, 4, NULL);
	  					
					 	object_number+=660;
					 						
						#ifdef _GUI
						//gMuggerTexture.render( pedxyz[0][0]*(mapWidth/256)-gMuggerTexture.getWidth()/2, pedxyz[0][01]*(mapHeight/256)-gMuggerTexture.getHeight()/2, NULL, degrees, NULL, flipType );
						#endif
						
						//if(i<6)
						//{
							pedxyz[i][0]=pedx;
							pedxyz[i][1]=pedy;
							
							int pedcar_addr;
							pedcar_addr=pedstruct+364;
							ReadProcessMemory(hProcess, (LPVOID)pedcar_addr,&pedcar, 4, NULL);
							
							if(pedcar) //in vehicle
							{
								int pedcar_sprite;
								int pedcar_sprite_addr=pedcar+88;
								ReadProcessMemory(hProcess, (LPVOID)pedcar_sprite_addr,&pedcar_sprite, 4, NULL);
								int pedcar_angle_addr=pedcar_sprite+88;
								int pedcar_angle;
								ReadProcessMemory(hProcess, (LPVOID)pedcar_angle_addr,&angle[i], 2, NULL);
								p1angle=angle[i];
								angle[i] = angle[i]/4;
		  						angle[i] = 360-angle[i];
		  						//printf("pedcar angle: %d\n",p2angle);
							}
							else
							{
								ReadProcessMemory(hProcess, (LPVOID)ped_angle_addr,&angle[i], 2, NULL);
		  						p1angle=angle[i];
								angle[i] = angle[i]/4;
	  							angle[i] = 360-angle[i];
							}	
							
							
						//}				
						#ifdef _GUI
						
						
						if(pedxyz[i][0]==oldpedxyz[i][0] && angle[i]==oldangle[i] && pedxyz[i][1]==oldpedxyz[i][1])
						{
							//thingschanged=false;
						}
						else
						{
							//printf("things changed!\n");
							thingschanged=true;				
							oldpedxyz[i][0]=pedx;	
							oldpedxyz[i][1]=pedy;
							oldangle[i]=angle[i];
						}
						
						if(thingschanged)
						{	
							if(!sektor || i>1)
							{
								switch (pedremap)
								{
								case 10:
								  //gArrowTexture[i].setColor(172, 34, 36 ); //red  
								  gArrowTexture[i].setColor(255, 0, 0 ); //red  
								  break;
								case 9:
								  //gArrowTexture[i].setColor(196, 126, 60 ); //orange
								  gArrowTexture[i].setColor(255, 128, 0 ); //orange
								  break;
								case 7:
								  //gArrowTexture[i].setColor(196, 142, 4 ); //yellow
								  gArrowTexture[i].setColor(255, 255, 0 ); //yellow
								  break;
								case 11:
								  gArrowTexture[i].setColor(0, 255, 0); //green
								  break;
								case 6:
								  gArrowTexture[i].setColor(0, 0, 255 ); //blue
								  break;
								case 13:
								  gArrowTexture[i].setColor(128, 0, 255 ); //purple
								  break;
								default:
								  gArrowTexture[i].setColor(255, 255, 255 ); //white
								  break;
								}
							}
							
							gArrowTexture[i].render( pedxyz[i][0]*(mapWidth/256)-gArrowTexture[0].getWidth()/2, pedxyz[i][1]*(mapHeight/256)-gArrowTexture[i].getHeight()/2, NULL, angle[i], NULL, flipType );
						}
						#endif
					}
													
					/*
					#ifdef _GUI	
					if(occupation==15)
						gMuggerTexture.render( pedxyz[0][0]*(mapWidth/256)-gCarThiefTexture.getWidth()/2, pedxyz[0][01]*(mapHeight/256)-gCarThiefTexture.getHeight()/2, NULL, degrees, NULL, flipType );
						
					if(occupation==16)
						gCarThiefTexture.render( pedxyz[0][0]*(mapWidth/256)-gCarThiefTexture.getWidth()/2, pedxyz[0][01]*(mapHeight/256)-gCarThiefTexture.getHeight()/2, NULL, degrees, NULL, flipType );
					#endif
					*/
					
					sprintf ( c1x, "%d", pedxyz[0][0] ); 
					sprintf ( c1y, "%d", pedxyz[0][1] );
					sprintf ( c2x, "%d", pedxyz[1][0] ); 
					sprintf ( c2y, "%d", pedxyz[1][1] ); 
					
					
					
					
					
					
					
					#ifndef _GUI
					if(SDL_GetTicks()>=prevTicks+100)
					{
						prevTicks = SDL_GetTicks();
						string strjpg = gmpfile;
						//remove extension from gmp file for jpg filename
						if(strjpg.size()>4)
						{
							unsigned sz = strjpg.size()-4;
					  		strjpg.resize (sz);
					  		strjpg+=".jpg";
						}
						else
						{
							strjpg="bil.jpg";
						}
						
						string curl="curl.exe";
						curl=curl + " \"http://127.0.0.1/gta2radar/radar.php?x=" + c1x + "&y=" + c1y + "&a=" + cangle + "&x2=" + c2x + "&y2=" + c2y + "&a2=" + c2a + "&bg=" + strjpg.c_str() + "\"";
						
						if(prevcurl!=curl)
						{
							prevcurl=curl;
							printf("Exec: %s\n",curl.c_str());	
							system(curl.c_str());
						}
					}
					#endif
					
					//gArrowTexture.render( ( SCREEN_WIDTH - gArrowTexture.getWidth() ) / 2, ( SCREEN_HEIGHT - gArrowTexture.getHeight() ) / 2, NULL, degrees, NULL, flipType );
					//gArrowTexture.render( p1x, p1y , NULL, degrees, NULL, flipType );
					
					//some dummy cars
					//car01struct_ptr = gta2base_addr+0x1F4CA0;
					//car01struct_offset = pedcar;
					/*
					for(int i = 0; i <200; i++) {
						//Objects
		      			ReadProcessMemory(hProcess, (LPVOID)car01struct_ptr,&car01struct_offset, 4, NULL);
						car01struct_offset+=0x4;
						ReadProcessMemory(hProcess, (LPVOID)car01struct_offset,&car01struct, 4, NULL);
						int x=i*188;
						car01struct+=x;
						car01struct+=88;
						ReadProcessMemory(hProcess, (LPVOID)car01struct,&car01sprite, 4, NULL);
						car01sprite+=48;
						ReadProcessMemory(hProcess, (LPVOID)car01sprite,&car01_x, 4, NULL);
						car01sprite+=4;
						ReadProcessMemory(hProcess, (LPVOID)car01sprite,&car01_y, 4, NULL);
						
						if(car01_x!=0){
						car01_x/=16384;
						car01_y/=16384;
						
						printf("car01 x: %d\n",car01_x);
						printf("car01 y: %d\n",car01_y);	
		
						gCarTexture.render( car01_x*(mapWidth/256)-gCarTexture.getWidth()/2, car01_y*(mapHeight/256)-gCarTexture.getHeight()/2, NULL, p1angle, NULL, flipType );
						}
					}
					*/
					
					#ifdef _gparkedcars
					object01_ptr = gta2base_addr+0x1E86D0; //parked cars and trains
					for(int i = 0; i <200; i++)
					{
						//Objects
		      			ReadProcessMemory(hProcess, (LPVOID)object01_ptr,&object01, 4, NULL);
						object01+=0x14;
						ReadProcessMemory(hProcess, (LPVOID)object01,&object01_x, 4, NULL);
						object01+=4;
						ReadProcessMemory(hProcess, (LPVOID)object01,&object01_y, 4, NULL);
						
						if(object01_x==p1x and object01_y==0)
						{
							//printf("object addr: %d\n",object01);
						}
						
						fParkedX =(float) object01_x/16384;
						fParkedY =(float) object01_y/16384;
						
						#ifdef _GUI
						gCarTexture.render( fParkedX*(mapWidth/256)-gCarTexture.getWidth()/2, fParkedY*(mapHeight/256)-gCarTexture.getHeight()/2, NULL, degrees, NULL, flipType );
						#endif
						object01_ptr+=0x10;
					}
					#endif
						
					int w;
					int h;
					float fw=w;
					float fh=h;
					SDL_GetWindowSize(gWindow,&w,&h);
					if(w!=oldWindowWidth || h!=oldWindowHeight)
					{
						oldWindowWidth=w;
						oldWindowHeight=h;
						thingschanged=true;
						//printf("window changed\n");
					}
					
					#ifdef _GUI
					//if(thingschanged)
					{
						if(SDL_GetTicks()>=prevTicks+1)
						{
						
						prevTicks = SDL_GetTicks();
	
						float scalex;
						float scaley;
						//Resize screen
						scalex=fw/gMapTexture.getWidth();
						scaley=fh/gMapTexture.getHeight();
						SDL_RenderSetScale(gRenderer,scalex,scaley);
						SDL_RenderPresent( gRenderer );
						}
					}
					#endif
				}
				else
				{
				//cout << "Radar disabled\n";	
				}
				CloseHandle(hProcess);
			}
		}
	#ifdef _GUI
	}
	
	//Free resources and close SDL
	close();
	#endif

	return 0;
}

#ifdef _GUI
void loadIcons(){
	gSaveTexture.loadFromFile( "images\\saveicon.png" );
	//gCopTexture.loadFromFile( "images\\cophead.png" );
	//gMuggerTexture.loadFromFile( "mugger.png" );
	//gCarThiefTexture.loadFromFile( "carthief.png" );
	gCarTexture.loadFromFile( "car.png" );
	//gTokenTexture.loadFromFile("images\\token.png"); // You can find gta2 tokens/icons by searching for hex string 0A0100000000000002
	gArrowTexture[0].loadFromFile( "images\\arrow.png" );
	//gArrowTexture[1].loadFromFile( "images\\vike.png" );
	//gArrowTexture[2].loadFromFile( "images\\cophead.png" );
	gArrowTexture[1].loadFromFile( "images\\arrow.png" );
	gArrowTexture[2].loadFromFile( "images\\arrow.png" );
	gArrowTexture[3].loadFromFile( "images\\arrow.png" );
	gArrowTexture[4].loadFromFile( "images\\arrow.png" );
	gArrowTexture[5].loadFromFile( "images\\arrow.png" );
	
	for(int i = 1; i <300; i++)
	{
		string filepath = "images\\" + to_string(i) + ".png";
		gObjectTexture[i].loadFromFile(filepath);
		
	}

	return;
}
#endif
