#ifndef TE_BANNER_H
#define TE_BANNER_H

void TE_ShowBanner(char *text, int frames);
int TE_TryShowBanner(char *text, int frames);

void TE_Banner_Install();
void TE_Banner_MatchLoaded();
void TE_Banner_MatchExit();
void TE_Banner_Frame();

#endif