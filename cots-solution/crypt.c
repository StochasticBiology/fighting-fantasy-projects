// simulation of champskees' best path through Crypt of the Sorcerer from
// https://fightingfantazine.proboards.com/thread/106/26-crypt-sorcerer-solution

#include <stdio.h>
#include <stdlib.h>

// verbose flags for extended output
#define VERBOSE 0                // outputs player status throughout
#define VERBOSE_COMBAT 0         // outputs combat progress

// random number call
#define RND drand48()

// housekeeping constants -- length of strings describing book stages, number of stages, and number of simulations to run
#define LEN 100
#define NSTAGE 28
#define NSIM 1e5

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
  int golem;
  int razaak;
  int dservant;
  int ratman;
  int cavetroll;
  int useluck;
  int ironeater;
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

void Heal(Player *Hero)
{
  int tots;
  tots = (int)((Hero->i_stamina-Hero->stamina)/4);
  if(tots > Hero->food) tots = Hero->food;

  dStamina(Hero, 4*tots);
  Hero->food -= tots;
}


void Die(Player *Hero)
{
  dStamina(Hero, -Hero->stamina);
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
  int opp_rounds;
  int hero_rounds;
  int round;
  
  // loop until opponent dies (remember we simulate until we finish the book, just remembering if we died or not along the way)
  opp_rounds = hero_rounds = round = 0;
  while(opp_stamina > 0 && Hero->stamina > 0)
    {
      // compute attack strengths
      hero_as = Hero->skill + D6() + D6();
      opp_as = opp_skill + D6() + D6();
      if(F.ratman && round == 0) hero_as--;
      
      // if we lose this round
      if(hero_as < opp_as)
	{
	  opp_rounds++;
	  hero_rounds = 0;
	  if(F.ironeater) dSkill(Hero, -1);
	  dmg = -2;
	  // if we've lost this round and our stamina is <= 2, test luck regardless of luck (alternative is certain death) to try and mitigate damage
	  // this would normally be only worth it for stamina exactly 2, but there's a chance our shield plus a luck test will save us even at stamina 1
	  if(Hero->stamina <= 2)
	    {
	      if(testLuck(Hero))
		dmg++;
	      else
		dmg--;
	    }
	  dStamina(Hero, dmg);
	  if(F.razaak == 1 && opp_rounds == 2) dStamina(Hero, -Hero->stamina);
	}
      if(hero_as > opp_as)
	{
	  hero_rounds++;
	  opp_rounds = 0;
	  if(F.dservant && hero_rounds == 2) opp_stamina = 0;
	  // try using luck if we want to and have an expectation of doing better
	  if(opp_stamina > 2 && F.useluck && Hero->luck >= 6)
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
      if(F.golem == 1 && opp_stamina > 0)
	{
	  if(D6() == 1) Die(Hero);
	}
      if(F.cavetroll)
	{
	  hero_as = 10 + D6() + D6();
	  opp_as = opp_skill + D6() + D6();
	  if(hero_as > opp_as) opp_stamina -= 2;
	  hero_as = 9 + D6() + D6();
	  opp_as = opp_skill + D6() + D6();
	  if(hero_as > opp_as) opp_stamina -= 2;
	}
      round++;
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
  int r;
  
  // populate list of stage names so we can see where we die
  sprintf(&stagenames[LEN*0], "flies");
  sprintf(&stagenames[LEN*1], "rad-hulks");
  sprintf(&stagenames[LEN*2], "chameleonites");
  sprintf(&stagenames[LEN*3], "bonekeeper");
  sprintf(&stagenames[LEN*4], "goblins");
  sprintf(&stagenames[LEN*5], "wood demon");
  sprintf(&stagenames[LEN*6], "werewolf");
  sprintf(&stagenames[LEN*7], "griffin");
  sprintf(&stagenames[LEN*8], "clay golem");
  sprintf(&stagenames[LEN*9], "centaur");
  sprintf(&stagenames[LEN*10], "giant boulder");
  sprintf(&stagenames[LEN*11], "giant combat");
  sprintf(&stagenames[LEN*12], "demonspawn");
  sprintf(&stagenames[LEN*13], "skeletons");
  sprintf(&stagenames[LEN*14], "lightning");
  sprintf(&stagenames[LEN*15], "doragars");
  sprintf(&stagenames[LEN*16], "iron eater");
  sprintf(&stagenames[LEN*17], "trapdoor");
  sprintf(&stagenames[LEN*18], "cave troll");
  sprintf(&stagenames[LEN*19], "ice ghosts");
  sprintf(&stagenames[LEN*20], "gargantis claw");
  sprintf(&stagenames[LEN*21], "rat man");
  sprintf(&stagenames[LEN*22], "hobgoblin");
  sprintf(&stagenames[LEN*23], "hg arrows");
  sprintf(&stagenames[LEN*24], "d servant");
  sprintf(&stagenames[LEN*25], "zombie");
  sprintf(&stagenames[LEN*26], "razaak");
  sprintf(&stagenames[LEN*27], "epilogue");


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
      Hero.i_skill = Hero.skill = 12; 
      Hero.i_stamina = Hero.stamina = 24; 
      Hero.i_luck = Hero.luck = 12; 
      Hero.food = 5; 
      Hero.died = 0;
      Hero.potion = 0;
      F.golem = 0; F.razaak = 0; F.dservant = 0; F.ratman = 0; F.cavetroll = 0; F.useluck = 0; F.ironeater = 0;


      // begin simulation (comments give paragraphs)
      // 1-146-249 (needle flies)
      r = D6();
      if(r >= 5) Die(&Hero);
      else if(r >= 3) { dStamina(&Hero, -6); dSkill(&Hero, -2); Hero.food--; }
      else { dStamina(&Hero, -4); Hero.food--; }
      Output(Hero, 0, &dstage);
      Heal(&Hero);
      
      // 185-97-310-45-143 (rad-hulks)
      Combat(&Hero, 10, 5, F);
      Combat(&Hero, 10, 6, F);
      Output(Hero, 1, &dstage);
      Heal(&Hero);
	    
      // 87-295-328 (chainmail)
      dSkill(&Hero, 1);
      // 235-284-33-52 (nugget, knife, holy water)
      dStamina(&Hero, 1);
      // 352 (crystal of sanity)-137- (chameleonites)
      if(!testSkill(Hero))
	{
	  if(!testLuck(&Hero)) Die(&Hero); else dStamina(&Hero, -1);
	  Combat(&Hero, 7, 7, F);
	  Combat(&Hero, 6, 6, F);
	  Combat(&Hero, 7, 5, F);
	}
      else
	{
	  Combat(&Hero, 7-2, 7, F);
	  Combat(&Hero, 6-2, 6, F);
	  Combat(&Hero, 7-2, 5, F);
	}
      Output(Hero, 2, &dstage);
      Heal(&Hero);
	    
      // 356-180-70-134 (bonekeeper)
      Combat(&Hero, 5, 6, F);
      dLuck(&Hero, -3);
      Output(Hero, 3, &dstage);
      Heal(&Hero);
	    
      // 287-265 (skull ring)- 369-306 [note granite door 184]-324-376
      dStamina(&Hero, 1);
      // 391-60 (goblins)
      Combat(&Hero, 5, 5, F);
      Combat(&Hero, 5, 6, F);
      Combat(&Hero, 6, 5, F);
      Combat(&Hero, 6, 6, F);
      Output(Hero, 4, &dstage);
      Heal(&Hero);
	    
      // 116 (2 gp, cracked mirror)- 140
      dStamina(&Hero, 2);
      // 292-203-365 (wood demon)
      if(!testSkill(Hero))
	{
	  Combat(&Hero, 11, 10, F);
	}
      else
	{
	  Combat(&Hero, 9, 10, F);
	}
      Output(Hero, 5, &dstage);
      Heal(&Hero);
	    
      // 89-319-360 [note rod 37] (silver rod)- 175 (4 gp, candle)-268 [note howling tunnels]-221-168-302-194-252 (werewolf)
      Combat(&Hero, 8, 9, F);
      Output(Hero, 6, &dstage);
      Heal(&Hero);
	    
      // 36-178 (griffin)
      Combat(&Hero, 10, 10, F);
      Output(Hero, 7, &dstage);
      Heal(&Hero);
	    
      // 230-358-131 (shield)
      dSkill(&Hero, 1);
      dLuck(&Hero, 1);
      // 29-205-299 (clay golem)
      F.golem = 1; F.useluck = 0;
      Combat(&Hero, 8, 9, F);
      F.golem = 0; F.useluck = 0;
      Output(Hero, 8, &dstage);
      Heal(&Hero);
	    
      // 362-102 [ring]-339 [razaak sword]-165 (centaur)
      Combat(&Hero, 10, 10, F);
      Output(Hero, 9, &dstage);
      Heal(&Hero);
	    
      // 289-304-127-258-48
      dStamina(&Hero, D6()+2);
      // 382-118
      if(D6() <= 2) Die(&Hero);
      Output(Hero, 10, &dstage);
      Heal(&Hero);
	    
      // 173 (hill giant)
      Combat(&Hero, 9, 10, F);
      Output(Hero, 11, &dstage);
      Heal(&Hero);
	    
      // 282 (1gp)-241-367 [immune to fire]
      dLuck(&Hero, 1);
      // 80-25-192-211 [parchment 66]-386-67-183-308-103
      dStamina(&Hero, 2);
      // 327-148-237 [suma 11]-377 (demonspawn)
      Combat(&Hero, 6, 6, F);
      Output(Hero, 12, &dstage);
      Heal(&Hero);
	    
      // 278-157
      dStamina(&Hero, D6()+2);
      Hero.food = 5;
      // 244
      dLuck(&Hero, 1);
      // 139 (skeletons)
      Combat(&Hero, 6, 5, F);
      Combat(&Hero, 6, 6, F);
      Combat(&Hero, 5, 6, F);
      Output(Hero, 13, &dstage);
      Heal(&Hero);
	    
      // 290 [note tamal 108]-389
      if(!testLuck(&Hero))
	{
	  dStamina(&Hero, D6()+3);
	  dSkill(&Hero, -1);
	}
      Output(Hero, 14, &dstage);
      Heal(&Hero);
	    
      // 300 [rod 13]-50 [rod of paralysis]
      dLuck(&Hero, 1);
      // 234-316
      dStamina(&Hero, 2);
      // 353-135
      if(!testLuck(&Hero))
	{
	  // 30
	}
      else
	{
	  // 59
	}
      // 90 (doragars)
      Combat(&Hero, 9, 9, F);
      Combat(&Hero, 9, 10, F);
      Output(Hero, 15, &dstage);
      Heal(&Hero);
	    
      // 144 [bronze key]-398-214-256 [note zombie 5]-338-222
      if(D6() < 6)
	{
	  dSkill(&Hero, -1);
	  F.ironeater = 1;
	  Combat(&Hero, 4, 5, F);
	  F.ironeater = 0;
	}
      Output(Hero, 16, &dstage);
      Heal(&Hero);
	    
      // 151-281
      if(!testLuck(&Hero))
	{
	  dStamina(&Hero, -D6());
	}
      Output(Hero, 17, &dstage);
      Heal(&Hero);
 
      // 79 (cave troll) 
      F.cavetroll = 1;
      Combat(&Hero, 8, 9, F);
      F.cavetroll = 0;
      Output(Hero, 18, &dstage);
      Heal(&Hero);
 
      // 19-366-104-266- [note hobbit ears 119]-40-315
      if(!(testSkill(Hero) && testSkill(Hero) && testSkill(Hero))) Die(&Hero);
      Output(Hero, 19, &dstage);
      Heal(&Hero);
 
      // 72-245-56-349
      dStamina(&Hero, -2);
      if(!testSkill(Hero)) Die(&Hero);
      Output(Hero, 20, &dstage);
      Heal(&Hero);
 
      // 88-193-375-128-38-311 (gargantis horn)
      if(!testLuck(&Hero))
	{
	  // 138
	  if(!testLuck(&Hero))
	    {
	      // 359
	      if(D6() <= 3) dStamina(&Hero, -2);
	      Combat(&Hero, 5, 6, F);
	    }
	  else
	    {
	      // 274-238
	      F.ratman = 1;
	      Combat(&Hero, 5, 6, F);
	      F.ratman = 0;
	    }
	  dStamina(&Hero, 1);
	}
      Output(Hero, 21, &dstage);
      Heal(&Hero);
 
      // 54-279
      if(!testSkill(Hero))
	{
	  if(!testSkill(Hero)) Die(&Hero);
	}
      Output(Hero, 22, &dstage);
      Heal(&Hero);
 
      // 253-226;
      r = D6();
      for(j = 0; j < r; j++)
	{
	  if(D6() == 6) dStamina(&Hero, -2);
	}
      Output(Hero, 23, &dstage);
      Heal(&Hero);
 
      // 294-198 (note warhammer 35)-152-[11]-28
      dStamina(&Hero, 2);
      // 167-96-179-336 (note hamakei)-313-162-81 (d servant)
      F.dservant = 1;
      Combat(&Hero, 8, 7, F);
      F.dservant = 0;
      Output(Hero, 24, &dstage);
      Heal(&Hero);
 
      // 93 (robe) -181-396-297-264-156-191-101-41 (zombie)
      Combat(&Hero, 6, 6-D6(), F);     
      Output(Hero, 25, &dstage);
      Heal(&Hero);
 
      // 74 (tag 283)-199-26-[283]-189-364-[35]-[119]-[5]-[108]-[184]-276-326-351-[66]-271 (razaak)
      F.razaak = 1;  F.useluck = 12;
      Combat(&Hero, 12, 20, F);
      Output(Hero, 26, &dstage);
      Heal(&Hero);
 
      // 84-233
      dStamina(&Hero, -6);
      Output(Hero, 27, &dstage);
      // 400

      if(dstage != -1)
        deadhist[dstage]++;      
      dead += Hero.died;
    }
  // summarise death statistics
  //  printf("Overall success probability %.3f (%.3f%%)\n", 1.- (double)dead / NSIM, 100.*(1.- (double)dead / NSIM));
  printf("Initial stats %i/%i/%i\n", Hero.i_skill, Hero.i_stamina, Hero.i_luck);
  
  printf("Overall success probability %.3f%%\n", 100.*(1.- (double)dead / NSIM));
  int deadtilnow = NSIM;
 
  for(i = 0; i < NSTAGE; i++)
    {
      if(deadhist[i])
	{
	  //	printf("Stage %i (%s): %.2f%% simulations died (%.2f%% of all deaths, %.2f%% of those that made it here)\n", i, &stagenames[i*LEN], (double)deadhist[i]/NSIM*100., (double)deadhist[i]/dead*100., (double)deadhist[i]/deadtilnow*100);
	  printf("Stage %i (%s): %.2f%% simulations died (%.2f%% of those that made it here)\n", i, &stagenames[i*LEN], (double)deadhist[i]/NSIM*100., (double)deadhist[i]/deadtilnow*100);
	}
      deadtilnow -= deadhist[i];
    }

  return 0;
}

  
