// simulation of champskees' best path through The Warlock of Firetop Mountain
// path taken from https://fightingfantazine.proboards.com/thread/56/1-warlock-firetop-mountain-solution
// only change (I think!) is that where champskees fights the iron cyclops and minotaur, they have "(use up to 3 luck points to deal extra damage, shield to reduce damage taken)."
// but this can be counterproductive if luck < 6 because then the expected damage dealt drops below 2 -- so we only use luck if it's >= 6
// we also always test luck if we're about to receive a killing blow in combat

#include <stdio.h>
#include <stdlib.h>

// verbose flags for extended output
#define VERBOSE 0                // outputs player status throughout
#define VERBOSE_COMBAT 0         // outputs combat progress
#define CHAMPSKEES 0             // attempt to replicate champskees

// random number call
#define RND drand48()

// housekeeping constants -- length of strings describing book stages, number of stages, and number of simulations to run
#define LEN 100
#define NSTAGE 11
#define NSIM 5e5

// structure storing player info
typedef struct {
  int i_luck, i_skill, i_stamina;  // initial stats
  int luck, skill, stamina;        // current stats
  int food;                        // current provisions
  int potion;                      // current doses of strength potion
  int died;                        // whether the player has died
} Player;

// structure storing adventure-specific flags
typedef struct {
  int shield;                      // shield with a chance of reducing damage
  int useluck;                     // how much luck we can use in this combat
  int northbank;                   // whether we've reached the north bank of the river
} Flags;

// random die roll
int D6(void)
{
  return (int)(RND*6 + 1);
}

// luck test -- returns 1 if successful and 0 otherwise, and reduces luck by 1
int testLuck(Player *Hero)
{
  int l;

  l = Hero->luck;
  Hero->luck--;
  if(D6() + D6() <= l) return 1;
  return 0;
}

// skill test -- returns 1 if successful and 0 otherwise
int testSkill(Player Hero)
{
  if(D6() + D6() <= Hero.skill) return 1;
  return 0;
}

// stamina test -- returns 1 if successful and 0 otherwise
int testStamina(Player Hero)
{
  if(D6() + D6() <= Hero.stamina) return 1;
  return 0;
}

// change luck by "d" (positive or negative) unless this takes us above initial score
void dLuck(Player *Hero, int d)
{
  if(Hero->luck + d <= Hero->i_luck) Hero->luck += d;
  else Hero->luck = Hero->i_luck;
}

// change skill by "d" (positive or negative) unless this takes us above initial score
void dSkill(Player *Hero, int d)
{
  if(Hero->skill + d <= Hero->i_skill) Hero->skill += d;
  else Hero->skill = Hero->i_skill;
}

// change stamina by "d" (positive or negative) unless this takes us above initial score
// if this takes stamina below zero, either use potion or die
void dStamina(Player *Hero, int d)
{
  if(Hero->stamina + d <= Hero->i_stamina) Hero->stamina += d;
  else Hero->stamina = Hero->i_stamina;
  if(Hero->stamina <= 0)   // we will die here without potion
    {
      // if we have potion, use and decrement
      if(Hero->potion)     
	{
	  Hero->potion--;
	  if(VERBOSE)
	    printf("took potion here\n");
	  Hero->stamina = Hero->i_stamina;
	}
      // otherwise, we die. for convenience we continue the simulation, just recording that we have died
      else                 
	{
	  if(VERBOSE)
	    {
              if(Hero->died == 0)
	        printf("Died here!\n");
	    }
          Hero->died = 1;
	}
    }
}

// eat provisions if we have food and stamina is below initial score
void Eat(Player *Hero, int d)
{
  if(Hero->stamina < Hero->i_stamina && Hero->food > 0)
    {
      Hero->food--;
      dStamina(Hero, d);
    }
}

// combat simulation against an opponent with "opp_skill", "opp_stamina"
// "F" stores any features that modify combat (whether to use luck, shield)
void Combat(Player *Hero, int opp_skill, int opp_stamina, Flags F)
{
  int hero_as;
  int opp_as;
  int dmg;
  
  // loop until opponent dies (remember we simulate until we finish the book, just remembering if we died or not along the way)
  while(opp_stamina > 0)
    {
      // compute attack strengths
      hero_as = Hero->skill + D6() + D6();
      opp_as = opp_skill + D6() + D6();
      // if we lose this round
      if(hero_as < opp_as)
	{
	  // lose one stamina point if we have shield and roll a 6
	  if(F.shield == 1 && D6() == 6)
	    dmg = -1;
	  else
	    dmg = -2;
	  // if we've lost this round and our stamina is <= 2, test luck regardless of luck (alternative is certain death) to try and mitigate damage
	  // this would normally be only worth it for stamina exactly 2, but there's a chance our shield plus a luck test will save us even at stamina 1
	  if(Hero->stamina <= (CHAMPSKEES ? 0 : 2))
	    {
	      if(testLuck(Hero))
		dmg++;
	      else
		dmg--;
	    }
	  dStamina(Hero, dmg);
	}
      if(hero_as > opp_as)
	{
	  // try using luck if we want to and have an expectation of doing better
	  if(F.useluck && Hero->luck >= (CHAMPSKEES ? 0 : 6))
	    {
	      if(testLuck(Hero))
		opp_stamina -= 4;
	      else
		opp_stamina -= 1;
	      F.useluck--;
	    }
	  // otherwise, don't use luck
	  else opp_stamina -= 2;
	}
      if(VERBOSE_COMBAT)
        printf("%i vs %i | %i, %i vs %i, %i\n", hero_as, opp_as, Hero->skill, Hero->stamina, opp_skill, opp_stamina);
    }
}

// output state of the player
// record whether the player died at this stage
void Output(Player Hero, int j, int *dstage)
{
  if(VERBOSE)
     printf("Stage %i: %i/%i/%i (%c)\n", j, Hero.skill, Hero.stamina, Hero.luck, (Hero.died ? 'X' : ' '));
  if(*dstage == -1 && Hero.died == 1)
    *dstage = j;
}

// main simulation
int main(void)
{
  Player Hero;
  Flags F;
  int i, j;
  int dead;
  int tmp;
  int deadhist[NSTAGE];
  int dstage;
  char stagenames[NSTAGE*LEN];

  // populate list of stage names so we can see where we die
  sprintf(&stagenames[LEN*0], "guard");
  sprintf(&stagenames[LEN*1], "snake");
  sprintf(&stagenames[LEN*2], "orcs");
  sprintf(&stagenames[LEN*3], "iron cyclops");
  sprintf(&stagenames[LEN*4], "barbarian");
  sprintf(&stagenames[LEN*5], "swimming");
  sprintf(&stagenames[LEN*6], "piranhas 1");
  sprintf(&stagenames[LEN*7], "piranhas 2");
  sprintf(&stagenames[LEN*8], "sucker punch");
  sprintf(&stagenames[LEN*9], "zombies");
  sprintf(&stagenames[LEN*10], "minotaur");
  
  // initialise counter and stager of player deaths
  dead = 0;
  for(i = 0; i < NSTAGE; i++)
    deadhist[i] = 0;
  
  // run many instances of the simulation
  for(i = 0; i < NSIM; i++)
    {
      // initialise stage counters 
      j = 0;
      dstage = -1;
      // initialise hero stats
      Hero.i_skill = Hero.skill = 7; 
      Hero.i_stamina = Hero.stamina = 14; 
      Hero.i_luck = Hero.luck = 7; 
      Hero.food = 10; 
      Hero.died = 0;
      Hero.potion = 2;
      F.shield = 0; 
      F.useluck = 0;
      F.northbank = 0;

      // begin simulation (comments give paragraphs)
      // 1-71 (guard)
      if(!testLuck(&Hero))
	Combat(&Hero, 6, 5, F);
      Output(Hero, 0, &dstage);
      
      // 301-208-397-240 (snake)
      Combat(&Hero, 5, 2, F);
      Output(Hero, 1, &dstage);

      // 145 (get key 99)
      dLuck(&Hero, 1); 
      // 363-370-116 (fight orcs -- a -1 attack modifier to them models the +1 we get)
      Combat(&Hero, 5-1, 4, F); 
      Combat(&Hero, 5-1, 5, F);
      Output(Hero, 2, &dstage);
      
      // 378-296 (di maggio spell)-42-113-285-314-223-53 (try to force door; 155 get shield if successful)
      if(Hero.stamina > 1)
	{
	  if(testSkill(Hero)) F.shield = 1;
	  else Hero.stamina--;
	}
      // 300-303-128-58 (rest ye here)-15
      Eat(&Hero, 6); dSkill(&Hero, 1);
      // 367-323-255-193-338 [iron cyclops]
      F.useluck = 3; Combat(&Hero, 10, 10, F); F.useluck = 0;
      Output(Hero, 3, &dstage);
      
      // 75 (get key 111 and eye of cyclops)
      dLuck(&Hero, 3); 
      Eat(&Hero, 4); 
      // 93-8 (escape from barbarian)
      dStamina(&Hero, -2);
      Output(Hero, 4, &dstage);

      // 189-90 (food opp)
      Eat(&Hero, 4); 
      // 253-73-218 (river section)
      // while loop will quit if we die or reach the north bank
      while(F.northbank == 0 && Hero.stamina > 0)
	{
	  // swimming acts as a possible provider of unlimited food opportunities
	  // so swim if stamina is under max and we still have food
	  while(Hero.stamina < Hero.i_stamina && Hero.food > 1)
	    {
	      // 316 (will your stamina hold out)
	      if(testStamina(Hero))
		{
		  // 151
		  if(Hero.stamina < 7)
		    {
		      dStamina(&Hero, -1);
		      Output(Hero, 5, &dstage);
		      // 218 (^ back to south bank)
		    }
		  else
		    {
		      // 158 (piranhas)
		      Combat(&Hero, 5, 5, F);
		      Output(Hero, 6, &dstage);
		      Eat(&Hero, 4);
		      // 218 (^ back to south bank after food opp)
		    }
		}
	      else
		{
		  Eat(&Hero, 4);
		  // 218 (^ back to south bank after food opp)
		}
	    }
	  // if stamina is max or we're out of food, punt across
	  // 386-55
	  if((CHAMPSKEES && (D6()+D6() <= Hero.luck && D6()+D6() <= Hero.stamina)) || (!CHAMPSKEES && (tmp = D6()+D6()) <= Hero.luck && tmp <= Hero.stamina))
	    {
	      F.northbank = 1;
	    }
	  else
	    {
	      // 166
	      if(D6() > 4)
		{
		  // 158 (piranhas)
		  Combat(&Hero, 5, 5, F);
		  Output(Hero, 7, &dstage);
		  Eat(&Hero, 4);
		  // 218 (^ back to south bank after food opp)
		}
	      else
		{
		  // 218 (^ back to south bank)
		}
	    }
	}
      
      // 7-214-104-49 (sucker punched)
      dStamina(&Hero, -2);
      Output(Hero, 8, &dstage);
      
      // 122-282 (zombies)
      Combat(&Hero, 7, 6, F); dLuck(&Hero, 2); 
      Combat(&Hero, 6, 6, F); 
      Combat(&Hero, 6, 6, F); 
      Combat(&Hero, 6, 5, F);
      Output(Hero, 9, &dstage);
      
      // 115-313 (plunder corpse)
      dLuck(&Hero, 1);
      dSkill(&Hero, 1);
      // 221-27 (magic sword)
      Hero.i_skill += 2;
      dSkill(&Hero, 2);
      dLuck(&Hero, 2);
      // 319-81-205-380-37-11 (magic tools)
      dStamina(&Hero, 2);
      dSkill(&Hero, 1); 
      // 366-89-286-294 (plunder corpse)
      dLuck(&Hero, 1); 
      // 107-197-48-391-52-291-227-131 (share food)
      Eat(&Hero, 2); 
      // 291-52-354-308-54-179 (minotaur)
      F.useluck = 12; Combat(&Hero, 9, 9, F); F.useluck = 0;
      Output(Hero, 10, &dstage);
      
      // 258 (get key 111)
      dLuck(&Hero, 2);
      // 54-308-160-267-246-329-299-359-385-297-150-222-85-106 (food opp; dragon -- use di maggio spell)
      Eat(&Hero, 4); 
      // 126-26-371 (food opp)
      Eat(&Hero, 4); 
      dLuck(&Hero, 3); 
      // 274 (warlock)-356-358-105-382-396-242-139 (use keys 111+111+99)-321-169-400 (end)

      // if we died, record that we did, and where
      if(dstage != -1)
        deadhist[dstage]++;      
      dead += Hero.died;
    }

  // summarise death statistics
  printf("Overall success probability %.3f\n", 1.- (double)dead / NSIM);
  for(i = 0; i < NSTAGE; i++)
    {
      if(deadhist[i])
	printf("Stage %i (%s): %.2f%% simulations died (%.2f%% of all deaths)\n", i, &stagenames[i*LEN], (double)deadhist[i]/NSIM*100., (double)deadhist[i]/dead*100.);
    }

  return 0;
}

  
