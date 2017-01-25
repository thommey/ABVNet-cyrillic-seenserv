#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

// don't change the buffer size
#define CYRBUFER 512

//utf-8 cyrillic range
#define MIN_CYR_16 0xd080
#define MAX_CYR_16 0xd4af

//cp1251 cyrillic range
#define MIN_CYR_8  0x80
#define MAX_CYR_8  0xFF

#define END_OF_IT 0x00

/* 
 * function transalte utf8 to cp1251 bits
 */
unsigned char * utf8_to_cp1251(char *stext);

/* 
 * function transalte cp1251 to utf8 bits
 */
unsigned char * cp1251_to_utf8(char *stext);

/* 
 * character tables
 */
extern const unsigned char utf8[];
extern const unsigned char cp1251[];
// ------


typedef struct seensrv_ seensrv_t;

struct seensrv_
{
  char *nick;                   /* the IRC client's nickname  */
  char *user;                   /* the IRC client's username  */
  char *host;                   /* the IRC client's hostname  */
  char *real;                   /* the IRC client's realname  */
  
  bool fantasy;		            /* enable fantasy commands    */
  
  char *trigger;		        /* trigger, e.g. !, ` or .    */

  service_t *me;                /* our user_t struct          */
  
  unsigned int seenexpire;               /* SeenServ records expire in days */
};

seensrv_t seensrv;

typedef struct seen_db_ seen_db_t;

struct seen_db_
{
	char purp[100];
	char *host;
	unsigned int time_in;
	unsigned int time_out;
	char *reason;
	int recors;
	bool exactm;
	int starting;
	int method;
	int utf8str;
};

const unsigned char ToLowerTabCyr[] = {
	0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa,
	0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11, 0x12, 0x13, 0x14,
	0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
	0x1e, 0x1f,
	' ', '!', '"', '#', '$', '%', '&', 0x27, '(', ')',
	'*', '+', ',', '-', '.', '/',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	':', ';', '<', '=', '>', '?',
	'@', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
	'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
	't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~',
	'_',
	'`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
	'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
	't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~',
	0x7f,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
	0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
	0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9,
	0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9,
	0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
	
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
	0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
	0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
	
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
	0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
	0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

int strcicmp(char const *a, char const *b, seen_db_t *dbs)
{
    for (;; a++, b++) {
		int d;
		if (dbs->exactm == true)
		{
			d = *a - *b;
		} else {
			d = ToLowerTabCyr[(unsigned char)(*a)] - ToLowerTabCyr[(unsigned char)(*b)];
		}
        if (d != 0 || !*a)
            return d;
    }
}

bool match_seen_mask(char *first, char * second, seen_db_t *dbs)
{
    if (*first == '\0' && *second == '\0')
        return true;

    if (*first == '*' && *(first+1) != '\0' && *second == '\0')
        return false;
        
    if (*first == '?' || ToLowerTabCyr[(unsigned char)(*first)] == ToLowerTabCyr[(unsigned char)(*second)] && dbs->exactm == false || *first == *second && dbs->exactm == true)
        return match_seen_mask(first+1, second+1, dbs);

    if (*first == '*')
        return match_seen_mask(first+1, second, dbs) || match_seen_mask(first, second+1, dbs);
    return false;
}

char *time_ago_seen(time_t event, time_t basis)
{
	static char ret[128];
	int years, weeks, days, hours, minutes, seconds;
    //CURRTIME
	event = basis - event;
	years = weeks = days = hours = minutes = 0;

	while (event >= 60 * 60 * 24 * 365)
	{
		event -= 60 * 60 * 24 * 365;
		years++;
	}
	while (event >= 60 * 60 * 24 * 7)
	{
		event -= 60 * 60 * 24 * 7;
		weeks++;
	}
	while (event >= 60 * 60 * 24)
	{
		event -= 60 * 60 * 24;
		days++;
	}
	while (event >= 60 * 60)
	{
		event -= 60 * 60;
		hours++;
	}
	while (event >= 60)
	{
		event -= 60;
		minutes++;
	}

	seconds = event;

	if (years)
		snprintf(ret, sizeof(ret), "%dy %dw %dd", years, weeks, days);
	else if (weeks)
		snprintf(ret, sizeof(ret), "%dw %dd %dh", weeks, days, hours);
	else if (days)
		snprintf(ret, sizeof(ret), "%dd %dh %dm %ds", days, hours, minutes, seconds);
	else if (hours)
		snprintf(ret, sizeof(ret), "%dh %dm %ds", hours, minutes, seconds);
	else if (minutes)
		snprintf(ret, sizeof(ret), "%dm %ds", minutes, seconds);
	else
		snprintf(ret, sizeof(ret), "%ds", seconds);

	return ret;
}

static char *get_word(char *text_line, char separator, int iword)
{
	int nword = 0;
    char *word = (char*)malloc(sizeof(char)*(BUFSIZE+1));
    int i = 0;	
        while(1){
            while(*text_line != separator && *text_line != '\0' &&  *text_line != '\n')
            {
              if(iword == nword)
				  word[i++] = *text_line;
		  
              text_line++;
            }
			   
            if(*text_line == '\0') 
                break;

            text_line++;
            nword++;
        }
		   if (i == 0)
		   {
		     return NULL;	
		      } else {
			 word[i] = '\0';	  
		     return word;			
		   }
}

int get_record(FILE *in, FILE *out, seen_db_t *dbs)
{
	char line[BUFSIZE], newline[BUFSIZE], reason[BUFSIZE] = "", rreason[BUFSIZE] = "";
	char *host_s;
	int found_it = 0;
	rewind(in);
   while (fgets(line , sizeof(line) ,in ) != NULL)
    {
      if (strcicmp(dbs->host ,get_word(line,' ', 0), dbs) == 0)
      {
		  found_it = 1;
		  if (strcmp(dbs->purp, "P") == 0) {
          snprintf(newline, sizeof newline, "%s %s %d %s\n", dbs->host, get_word(line,' ', 1), dbs->time_out, dbs->reason);	  
		  }
		  if (strcmp(dbs->purp, "S") == 0) {
		   int s = 3;
		   host_s = get_word(line,' ', 3);	   
		   while (host_s != NULL)
            {
		       snprintf(rreason, sizeof rreason, " %s", host_s);
               strcat(reason, rreason);
               s++;			   
               host_s = get_word(line,' ', s);
             }
          snprintf(newline, sizeof newline, "%s %d %s%s\n", dbs->host, dbs->time_in, get_word(line,' ', 2), reason);
		  }
		  break;
      }
    }
	if (found_it == 1)
	{
	   fputs(newline, out);
	   return 1;
	  } else {
	   return 0;
	}
}


void seen_write_to_db(const char *nick, seen_db_t *dbs) {
     FILE *in, *out;
	char line[BUFSIZE], linebuf[BUFSIZE], newline[BUFSIZE], reason[BUFSIZE] = "", rreason[BUFSIZE] = "";
	char *host_s;
	char name[BUFSIZE];
    char name_temp[BUFSIZE];
	snprintf(name, BUFSIZE, "%s/%s", datadir, "seenserv.db");
	snprintf(name_temp, BUFSIZE, "%s/%s", datadir, "seenserv.temp");
	
	int found_it = 0;
	
	user_t *target_u;
	
   if (!strcmp(dbs->purp, "R") == 0) {
	 in = fopen(name, "r" );	
	out = fopen(name_temp, "w+" );

	if (get_record(in, out, dbs) == 1)
	{
	  found_it = 1;
	}
   rewind(in);
   while (fgets(line , sizeof(line) ,in ) != NULL)
    {
	  strcpy(linebuf, line);
      if (strcicmp(dbs->host ,get_word(line,' ', 0), dbs) == 0)
      {
       } else {
		  if (strlen(linebuf) > 0)
		   {
             target_u = user_find_named(get_word(linebuf,'!', 0));
		   if ((atoi(get_word(linebuf,' ', 1)) + seensrv.seenexpire * 24 * 60 * 60) > CURRTIME)
		     {
			   fputs(linebuf, out);  
		     } else if(target_u != NULL)
			 {
			   snprintf(reason, sizeof reason, "%s!%s@%s", target_u->nick, target_u->user, target_u->vhost); 
			   if(strcicmp(dbs->host ,reason, dbs) == 0)
			   {
				fputs(linebuf, out);
			   }				   
			 }
	       }
	    }
     }
	  if(found_it == 0)
	  {
		 snprintf(newline, sizeof newline, "%s %d 0 \n", dbs->host, dbs->time_in);
		 fputs(newline, out);
	  }
    fclose(in);
	fclose(out);
	remove(name);
	rename(name_temp, name);
   } else {
	char *result[9];
	char *hostc, *nick_s; 
	char lineto[BUFSIZE], fline[BUFSIZE], eline[BUFSIZE];
    int count = 0, scount = 0, current = 0, mcount = 0;
	myuser_t *mu;
	time_t time_login, time_singout;
	in = fopen( name, "r" );
     while (fgets(line , sizeof(line) ,in ) != NULL)
    {
	   strcpy(lineto, line);
	   nick_s = get_word(line,'!', 0);
      if (match_seen_mask(dbs->host, get_word(line,' ', 0), dbs) && dbs->method == 1 || match_seen_mask(dbs->host, nick_s, dbs) && dbs->method == 0 || strcicmp(dbs->host, nick_s, dbs) == 0)
      {
	       target_u = user_find_named(nick_s);
	      if (strcicmp(dbs->host, nick_s, dbs) == 0 && target_u)
	      {
            goto isonlinenow;
	      }		  
		  if(mcount >= (dbs->starting - 1))
		  {
		  if (count < dbs->recors)
		  {
			result[count] = (char*)malloc(1024*sizeof(char));
		    strcpy(result[count], lineto);
            current++;			
	      } 
          count++;
		 }
	    mcount++;
	  }
	  
	    if(scount > 100000)
		  {
			break;			
		  }
	       scount++;
    }
	fclose(in);
	
	if (scount > 100000)
	{
     notice("SeenServ", nick, "There are more than\2 100000\2 matches to your query; please refine it to see any output");	
	} else {
	  if (current == 0)
	  {
	    notice("SeenServ", nick, "No search results for \2%s", dbs->host);
	  }	else {
	   int i;
	   notice("SeenServ", nick, "There are \2%i\2 results in the database; displaying max \2%i\2 starting from number \2%i\2: ", (count + dbs->starting - 1 ), current, dbs->starting);
	   for (i = 0; i < current; i++)
	   {
		 int m = 3;
		 strcpy(eline, "");
		 strcpy(lineto, result[i]);
		 snprintf(fline, sizeof fline, "\2%s\2 ", get_word(lineto,'!', 0));	  
		 strcat(eline, fline);
		 snprintf(fline, sizeof fline, "(\2%s\2) ", get_word(get_word(lineto,' ', 0), '!', 1));
		 strcat(eline, fline);
				
		 time_login = atoi(get_word(lineto,' ', 1));

		 time_singout = atoi(get_word(lineto,' ', 2));
		 
		 if (time_singout != 0)
		 {
		 snprintf(fline, sizeof fline, "was online \2%s\2 ago ", time_ago_seen(time_singout, CURRTIME));
		 strcat(eline, fline);	
           if (time_login < time_singout)
		   {
		     snprintf(fline, sizeof fline, "for \2%s\2", time_ago_seen(time_login, time_singout));
		     strcat(eline, fline);				   
		   } else {
		     snprintf(fline, sizeof fline, "for \2%s\2", time_ago_seen(time_login, CURRTIME));
		     strcat(eline, fline);				   
		   }		 
		 } else {
		 snprintf(fline, sizeof fline, "was online \2%s\2 ago ", time_ago_seen(time_login, CURRTIME));
		 strcat(eline, fline);				 
		 }

 		 mu = myuser_find_ext(get_word(lineto,'!', 0));
		  if (mu != NULL)
			{
			  if(mu->lastlogin > time_login)
			  {
		         snprintf(fline, sizeof fline, " | NickServ: \2Yes\2 |");
		         strcat(eline, fline);					  
			  } else {
		         snprintf(fline, sizeof fline, " | NickServ: \2No\2 |");
		         strcat(eline, fline);					  
			  }
			} else {
		         snprintf(fline, sizeof fline, " | NickServ: \2No\2 |");
		         strcat(eline, fline);						
			}
			
			hostc = get_word(lineto,' ', 3);		
			while (hostc != NULL)
            {
			snprintf(fline, sizeof fline, " %s", hostc);
			strcat(eline, fline);
			m++;
			hostc = get_word(lineto,' ', m);
			}
	     notice("SeenServ", nick, "[%d] %s", (i + 1), eline);
	     free(result[i]);
	    }			  
	  }
	  goto endd;
	  isonlinenow:
	  notice("SeenServ", nick, "\2%s (%s@%s)\2 is currently online.", target_u->nick, target_u->user, target_u->vhost);
	  endd:;
	}
  }
}

/* 
 * match_utf_cyr match the utf-8 character cyrillic range
 */
 
int match_utf_cyr(char *stext)
{
    int i, m, s;
    i = 0; m = 0; s = 0;
	char text[CYRBUFER];
	memset(text, 0, CYRBUFER);
	strcpy(text, stext);
	for (i = 0 ; i <  sizeof text; i++) {
	  if ((unsigned char)(text[i]) != END_OF_IT) {
		if (s == 0) {
		if (((((unsigned char)text[i]<<8) | (unsigned char)text[i+1]) >= 0xd090) && ((((unsigned char)text[i]<<8) | (unsigned char)text[i+1]) <= 0xd18f)) {
          m++; s = 1;
	    } else if ((unsigned char)text[i] >= 0x80 && (unsigned char)text[i] <= 0xFF) {
		  //return FALSE;	
		} else if ((unsigned char)text[i] >= 0x21 && (unsigned char)text[i] <= 0x7E && (unsigned char)text[i] != 0x2a && (unsigned char)text[i] != 0x3f) {
		  //return FALSE;
		} 
	   } else {
		  if ((unsigned char)text[i] >= 0x21 && (unsigned char)text[i] <= 0x7E  && (unsigned char)text[i] != 0x2a && (unsigned char)text[i] != 0x3f) {
			  //return FALSE;
		  } 
	    s = 0;
	    }
	   } else {break;}
   }
       if (m > 1) {return TRUE;} else {return FALSE;}
}

/* 
 * cp1251_to_utf8 translate cyrillic range ONLY from cp1251 to utf-8
 */
 
unsigned char * cp1251_to_utf8(char *stext)
{
	int i, m, s;
	char text[CYRBUFER];
	static unsigned char output[CYRBUFER];
	memset(text, 0, CYRBUFER);
	strcpy(text, stext);
	s = 0;
	for (i = 0 ; i <  sizeof text; i++) {
		if ((unsigned char)text[i] >= MIN_CYR_8 && (unsigned char)text[i] <= MAX_CYR_8) {
            for (m = 0; m < strlen((char*)cp1251); m++) {
              if ((unsigned char)text[i] == cp1251[m]) {
				output[s] = utf8[m*2];
				output[s+1] = utf8[m*2+1];
				s+=2;
                // if the symbol does not exists in cp1251 do nothing
				goto searchOK;
			   }
		     }
		      searchOK:;
	        } else {
		    output[s] = text[i];
		    s++;
		 }
   }
	return output;
}

/* 
 * cp1251_to_utf8 translate cyrillic range ONLY from utf-8 to cp1251
 */
 
unsigned char * utf8_to_cp1251(char *stext)
{
	int i, m, s, sh;
	char text[CYRBUFER];
	static unsigned char output[CYRBUFER];
	memset(output, 0, CYRBUFER);
	memset(text, 0, CYRBUFER);
	strcpy(text, stext);
	sh = 0; s = 0;
	for (i = 0 ; i <  sizeof text; i++) {
		if ((unsigned char)(text[i]) != END_OF_IT) {
		if ((((unsigned char)text[i]<<8) | (unsigned char)text[i+1]) >= MIN_CYR_16 && (((unsigned char)text[i]<<8) | (unsigned char)text[i+1]) <= MAX_CYR_16) {
            for (m = 0; m < strlen((char*)utf8); m+= 2) {
              if ((unsigned char)text[i] == utf8[m] && (unsigned char)text[i+1] == utf8[m + 1]) {
				output[s] = cp1251[m/2];
				s++;
				sh = 1;
				// if the symbol does not exists in utf-8 do nothing
				goto searchOK;
			   }
		     }
		      searchOK:;
	        } else {
				if (sh == 0) {
				    output[s] = text[i];
				    s++;
				} else {
					sh = 0;
				}
		 }
	 } else {break;}
   }
	return output;
}

/* 
 * utf-8 cyrillic characters split by 8-bit
 * two 8-bit values is equal to one utf-8 hexadecimal character
 */
 
const unsigned char utf8[] = {
	   /*?*/       /*?*/       /*?*/       /*?*/
    0xd0, 0x82, 0xd0, 0x83, 0xd1, 0x93, 0xd0, 0x89,
       /*?*/       /*?*/       /*?*/       /*?*/
    0xd0, 0x8a, 0xd0, 0x8c, 0xd0, 0x8b, 0xd0, 0x8f,
       /*?*/       /*?*/       /*?*/       /*?*/
    0xd1, 0x92, 0xd1, 0x99, 0xd1, 0x9a, 0xd1, 0x9c,
       /*?*/       /*?*/       /*?*/       /*?*/
    0xd1, 0x9b, 0xd1, 0x9f, 0xd0, 0x8e, 0xd1, 0x9e,
       /*?*/       /*?*/       /*?*/       /*?*/
    0xd0, 0x88, 0xd2, 0x90, 0xd0, 0x81, 0xd0, 0x84,
       /*?*/       /*?*/       /*?*/       /*?*/
    0xd0, 0x87, 0xd0, 0x86, 0xd1, 0x96, 0xd2, 0x91,
       /*?*/       /*?*/       /*?*/       /*?*/
    0xd1, 0x91, 0xd1, 0x94, 0xd1, 0x98, 0xd1, 0x95,
       /*?*/       /*?*/       /*?*/       /*?*/
    0xd1, 0x95, 0xd1, 0x97, 0xd0, 0x90, 0xd0, 0x91,
       /*?*/       /*?*/       /*?*/       /*?*/
    0xd0, 0x92, 0xd0, 0x93, 0xd0, 0x94, 0xd0, 0x95,
       /*?*/       /*?*/       /*?*/       /*?*/
    0xd0, 0x96, 0xd0, 0x97, 0xd0, 0x98, 0xd0, 0x99,
       /*?*/       /*?*/       /*?*/       /*?*/
    0xd0, 0x9a, 0xd0, 0x9b, 0xd0, 0x9c, 0xd0, 0x9d,
       /*?*/       /*?*/       /*?*/       /*?*/
    0xd0, 0x9e, 0xd0, 0x9f, 0xd0, 0xa0, 0xd0, 0xa1,
       /*?*/       /*?*/       /*?*/       /*?*/
    0xd0, 0xa2, 0xd0, 0xa3, 0xd0, 0xa4, 0xd0, 0xa5,
       /*?*/       /*?*/       /*?*/       /*?*/
    0xd0, 0xa6, 0xd0, 0xa7, 0xd0, 0xa8, 0xd0, 0xa9,
       /*?*/       /*?*/       /*?*/       /*?*/
    0xd0, 0xaa, 0xd0, 0xab, 0xd0, 0xac, 0xd0, 0xad,
       /*?*/       /*?*/       /*?*/       /*?*/
    0xd0, 0xae, 0xd0, 0xaf, 0xd0, 0xb0, 0xd0, 0xb1,
       /*?*/       /*?*/       /*?*/       /*?*/
    0xd0, 0xb2, 0xd0, 0xb3, 0xd0, 0xb4, 0xd0, 0xb5,
       /*?*/       /*?*/       /*?*/       /*?*/
    0xd0, 0xb6, 0xd0, 0xb7, 0xd0, 0xb8, 0xd0, 0xb9,
       /*?*/       /*?*/       /*?*/       /*?*/
    0xd0, 0xba, 0xd0, 0xbb, 0xd0, 0xbc, 0xd0, 0xbd,
       /*?*/       /*?*/       /*?*/       /*?*/
    0xd0, 0xbe, 0xd0, 0xbf, 0xd1, 0x80, 0xd1, 0x81,
       /*?*/       /*?*/       /*?*/       /*?*/
    0xd1, 0x82, 0xd1, 0x83, 0xd1, 0x84, 0xd1, 0x85,
       /*?*/       /*?*/       /*?*/       /*?*/
    0xd1, 0x86, 0xd1, 0x87, 0xd1, 0x88, 0xd1, 0x89,
       /*?*/       /*?*/       /*?*/       /*?*/
    0xd1, 0x8a, 0xd1, 0x8b, 0xd1, 0x8c, 0xd1, 0x8d,
       /*?*/       /*?*/
    0xd1, 0x8e, 0xd1, 0x8f
};

/* 
 * cp1251 cyrillic characters in 8-bit
 */
 
const unsigned char cp1251[] = {
	/*?*/ /*?*/ /*?*/ /*?*/
    0x80, 0x81, 0x83, 0x8a,
    /*?*/ /*?*/ /*?*/ /*?*/
    0x8c, 0x8d, 0x8e, 0x8f,
    /*?*/ /*?*/ /*?*/ /*?*/
    0x90, 0x9a, 0x9c, 0x9d,
    /*?*/ /*?*/ /*?*/ /*?*/
    0x9e, 0x9f, 0xa1, 0xa2,
    /*?*/ /*?*/ /*?*/ /*?*/
    0xa3, 0xa5, 0xa8, 0xaa,
    /*?*/ /*?*/ /*?*/ /*?*/
    0xaf, 0xb2, 0xb3, 0xb4,
    /*?*/ /*?*/ /*?*/ /*?*/
    0xb8, 0xba, 0xbc, 0xbd,
    /*?*/ /*?*/ /*?*/ /*?*/
    0xbe, 0xbf, 0xc0, 0xc1,
    /*?*/ /*?*/ /*?*/ /*?*/
    0xc2, 0xc3, 0xc4, 0xc5,
    /*?*/ /*?*/ /*?*/ /*?*/
    0xc6, 0xc7, 0xc8, 0xc9,
    /*?*/ /*?*/ /*?*/ /*?*/
    0xca, 0xcb, 0xcc, 0xcd,
    /*?*/ /*?*/ /*?*/ /*?*/
    0xce, 0xcf, 0xd0, 0xd1,
    /*?*/ /*?*/ /*?*/ /*?*/
    0xd2, 0xd3, 0xd4, 0xd5,
    /*?*/ /*?*/ /*?*/ /*?*/
    0xd6, 0xd7, 0xd8, 0xd9,
    /*?*/ /*?*/ /*?*/ /*?*/
    0xda, 0xdb, 0xdc, 0xdd,
    /*?*/ /*?*/ /*?*/ /*?*/
    0xde, 0xdf, 0xe0, 0xe1,
    /*?*/ /*?*/ /*?*/ /*?*/
    0xe2, 0xe3, 0xe4, 0xe5,
    /*?*/ /*?*/ /*?*/ /*?*/
    0xe6, 0xe7, 0xe8, 0xe9,
    /*?*/ /*?*/ /*?*/ /*?*/
    0xea, 0xeb, 0xec, 0xed,
    /*?*/ /*?*/ /*?*/ /*?*/
    0xee, 0xef, 0xf0, 0xf1,
    /*?*/ /*?*/ /*?*/ /*?*/
    0xf2, 0xf3, 0xf4, 0xf5,
    /*?*/ /*?*/ /*?*/ /*?*/
    0xf6, 0xf7, 0xf8, 0xf9,
    /*?*/ /*?*/ /*?*/ /*?*/
    0xfa, 0xfb, 0xfc, 0xfd,
    /*?*/ /*?*/
    0xfe, 0xff
};
