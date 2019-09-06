#include "mmr.h"
#include "cdf.h" //table that saves a lot of time
#include <float.h>
#define DIST_ITER for(int x = 0; x!= DISTVARNO; x++)
void decay(double * dist, double H)
{
	//this increases the effect of subsequent updates
	DIST_ITER
		dist[x] = (dist[x] + H)/(H*DISTVARNO)+1;
}
double cdf_i(double x, double theta, double mu)
{
	int location = (((x-mu)/theta)*500)+5000;
	if ( location > 9870)
		return 1.0 - DBL_MIN;
	if (location < 0)
		return DBL_MIN;
	return cdf_const[location];
}
double wAverage(double * dist)
{
	//gets raw mmr from a player or team curve
	double res = 0;
	DIST_ITER
		res += x*dist[x];
	return res;
}

void iniUniform(double * p)
{
	//needed for a new player
	DIST_ITER
		p[x] = 1.0/DISTVARNO;
}
void wPartyDist(double ** p_in , int p_count, double * p_out)
{
	//init to 1
	DIST_ITER
		p_out[x]=1.0;
	//sets p_out to a joint distribution for the party
	double cumsum = 0.0;
	DIST_ITER
	{
		for(int player = 0; player!= p_count; player++)
		{
			p_out[x]*= p_in[player][x];
		};
		cumsum += p_out[x];
	};
	//normalize
	DIST_ITER
		p_out[x]/=cumsum;
}
void updateMMR(double * dist,int outcome, double delta)
{
	//outcome is nonzero if this player was on winning team
	double mmr_effective = wAverage(dist) + delta;
	double cumsum = 0;
	DIST_ITER
	{
		double p = cdf_i(x-mmr_effective, 1.0, 0.0);
		dist[x]*= outcome?1.0-p:p; 
		cumsum += dist[x];
	};
	//normalize
	DIST_ITER
		dist[x]/=cumsum;	
}
int processMatch(double ** p1, double** p2, int p_count) 
{
	//p1 is the winning party, the parties must be the same size
	double p1_dist [DISTVARNO];
	double p2_dist [DISTVARNO];
	wPartyDist(p1,p_count,p1_dist);
	wPartyDist(p2,p_count,p2_dist);
	double delta = wAverage(p1_dist) - wAverage(p2_dist);
	//update each player distribution based on the effective mmr (player mmr + delta
	for (int player = 0;player != p_count; player++)
	{
		//party 1
		updateMMR(p1[player],1,delta);
		//party 2
		updateMMR(p2[player],0,-delta);
	};
	return 0;
}


