/*********************************************************************************/
/* GPSTurbo                                                                      */
/*                                                                               */
/* Programmed by Kevin Pickell                                                   */
/*                                                                               */
/* http://www.scale18.com/cgi-bin/page/gpsturbo.html                             */
/*                                                                               */
/*    GPSTurbo is free software; you can redistribute it and/or modify           */
/*    it under the terms of the GNU General Public License as published by       */
/*    the Free Software Foundation; version 2.                                   */
/*                                                                               */
/*    GPSTurbo is distributed in the hope that it will be useful,                 */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of             */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              */
/*    GNU General Public License for more details.                               */
/*                                                                               */
/*    http://www.gnu.org/licenses/gpl.txt                                        */
/*                                                                               */
/*    You should have received a copy of the GNU General Public License          */
/*    along with GPSTurbo; if not, write to the Free Software                    */
/*    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */
/*                                                                               */
/*********************************************************************************/

#include "gpsturbo.h"

enum
{
TILESOURCE_SAT,
TILESOURCE_OVERLAY,
TILESOURCE_MAP,
TILESOURCE_TERRAIN,
TILESOURCE_NUM
};

kGUIString GGPXMap::m_mapver;
kGUIString GGPXMap::m_satmapver;
kGUIString GGPXMap::m_overlayver;
kGUIString GGPXMap::m_terver;
kGUIThread GGPXMap::m_checkthread;

void GGPXMap::InitVersions(void)
{
	m_mapver.SetString("m@113");
	m_satmapver.SetString("48");
	m_overlayver.SetString("h@113");
	m_terver.SetString("w2p.113");
}

void GGPXMap::LoadVersions(kGUIXMLItem *root)
{
	kGUIXMLItem *gmaproot;
	kGUIXMLItem *item;

	if(!root)
		return;

	gmaproot=root->Locate("gmap");
	if(!gmaproot)
		return;

	/* make sure exe version is the same, if not, then ignore */
	item=gmaproot->Locate("version");
	if(!strcmp(__DATE__ "/" __TIME__,item->GetValue()->GetString()))
	{
		gpx->Get(gmaproot,"mapver",&m_mapver);
		gpx->Get(gmaproot,"satmapver",&m_satmapver);
		gpx->Get(gmaproot,"overlayver",&m_overlayver);
		gpx->Get(gmaproot,"terver",&m_terver);
	}
}

void GGPXMap::SaveVersions(kGUIXMLItem *root)
{
	kGUIXMLItem *gmaproot;

	/* wait for update thread to finish just incase it is still busy */
	while(m_checkthread.GetActive());

	gmaproot=root->AddChild("gmap");
	gmaproot->AddParm("version",__DATE__ "/" __TIME__);
	gmaproot->AddChild("mapver",&m_mapver);
	gmaproot->AddChild("satmapver",&m_satmapver);
	gmaproot->AddChild("overlayver",&m_overlayver);
	gmaproot->AddChild("terver",&m_terver);
}

/* this is called to see if the tile versions have changed */

void GGPXMap::CheckVersions(void)
{
	m_checkthread.Start(0,GGPXMap::CheckVersionsThread);
}

/* this runs in it's own thread and checks to see if any tile versions are updated to newer ones */
void GGPXMap::CheckVersionsThread(void *unused)
{
	kGUIString url;
	kGUIDownloadEntry dle;
	DataHandle dh;
	int result;
	unsigned int i;
	unsigned char c;
	kGUIString s;

	url.SetString("http://maps.google.com");
	dh.SetMemory();
	result=dle.DownLoad(&dh,&url);

	if(result==DOWNLOAD_OK)
	{
		/* scan html file and look inside Javascript for map tile versions */
		dh.Open();
		if(dh.Seek("pageArgs"))
		{
			for(i=0;i<4;++i)
			{
				s.Clear();
				if(dh.Seek("http://")==false)
					break;
				if(dh.Seek("\\x3d")==false)
					break;
				c=dh.ReadChar();
				while(c!='\\' && dh.Eof()==false)
				{
					s.Append(c);
					c=dh.ReadChar();
				}
				if(dh.Seek("http://")==false)
					break;
				switch(i)
				{
				case 0:
					m_mapver.SetString(&s);
				break;
				case 1:
					m_satmapver.SetString(&s);
				break;
				case 2:
					m_overlayver.SetString(&s);
				break;
				case 3:
					m_terver.SetString(&s);
				break;
				}
				kGUI::Trace("Map version string='%s'\n",s.GetString());
			}
		}
		dh.Close();
	}
	m_checkthread.Close(true);
}

void GGPXMap::GenerateURL(unsigned int type,kGUIString *url,kGUIString *hstr,int *serverid,int tx,int ty,int zoom)
{
	*serverid=(tx+ty)&3;

	switch(type)
	{
	case TILESOURCE_SAT:
	{
		int ty2;
		int bit;
		int c;
		static char xref[]={"qrts"};

		hstr->Clear();
		ty2=ty|(1<<zoom);
		bit=1<<zoom;
		while(bit)
		{
			if(ty2&bit)
				c=2;
			else
				c=0;
			if(tx&bit)
				++c;
			hstr->Append(xref[c]);
			bit>>=1;
		}

		url->Sprintf("http://khm%d.google.com/kh/v=%s&hl=en&t=%s",*serverid,m_satmapver.GetString(),hstr->GetString());
	}
	break;
	case TILESOURCE_OVERLAY:
		url->Sprintf("http://mt%d.google.com/vt/lyrs=%s&hl=en&x=%d&y=%d&zoom=%d",*serverid,m_overlayver.GetString(),tx,ty,17-zoom);
	break;
	case TILESOURCE_MAP:
		url->Sprintf("http://mt%d.google.com/vt/lyrs=%s&hl=en&x=%d&y=%d&zoom=%d",*serverid,m_mapver.GetString(),tx,ty,17-zoom);
	break;
	case TILESOURCE_TERRAIN:
		url->Sprintf("http://mt%d.google.com/vt/v=%s&hl=en&x=%d&y=%d&zoom=%d",*serverid,m_terver.GetString(),tx,ty,17-zoom);
	break;
	}
}

GGPXMap::GGPXMap(int type)
{	
	int i,y;
	double c;
	kGUIDate d;

	d.SetToday();
	y=d.GetYear();

	m_type=type;	/* 3 different types */

	for(i=0;i<MAXDLS;++i)
		m_dles[i].SetOwner(this);

	/* list of unsucessfully downloaded tiles */
	m_badurls.Init(12,0);

	SetZoomLevels(MINGZOOM,MAXGZOOM);
	SetTileSize(256,256);

	m_copyright.Sprintf("%c %d Google, Imagery %c %d, TerraMetrics, NASA - Map Data %c %d NAVTEQ%c",169,y,169,y,169,y,153);

	for(i=0;i<MAXGZOOM;++i)
	{
		c=(double)(1<<i);
		m_pixelsPerLonDegree[i]=c/360.0f;
  		m_negpixelsPerLonRadian[i] = -(c / (2.0f*3.1415926535f));
		m_bitmapOrigo[i]=c/2.0f;
		m_numTiles[i] = 1<<i;
		SetSize(i,m_numTiles[i]*256,m_numTiles[i]*256);
	}
}

/* online flag has changed, try downloading missing tiles again */
void GGPXMap::ResetOnline(void)
{
	m_badurls.Init(12,0);
}

GGPXMap::~GGPXMap()
{
	int i;
	bool busy;

	/* kill the check thread if it is still active */
	if(m_checkthread.GetActive()==true)
		m_checkthread.Close(true);

	for(i=0;i<MAXDLS;++i)
		m_dles[i].Abort();	/* cancel any pending downloads */

	/* wait for all to finish */
	do
	{
		busy=false;
		for(i=0;i<MAXDLS;++i)
		{
			if(m_dles[i].GetAsyncActive()==true)
				busy=true;
		}
		if(busy)
			kGUI::Sleep(1);
	}while(busy);
}

void GGPXMap::UpdateStatus(void)
{
	int i,numloading;

	m_status.Clear();

	if(m_checkthread.GetActive()==true)
		m_status.Sprintf("Checking Map Tile Versions...");
	else
	{
		numloading=0;
		for(i=0;i<MAXDLS;++i)
		{
			if(m_dles[i].GetAsyncActive()==true)
				++numloading;
		}
		if(numloading)
			m_status.Sprintf("Downloading %d tiles",numloading);
	}
}

void GGPXMap::ToMap(GPXCoord *c,int *sx,int *sy)
{
	double e;
	int z=GetZoom();

  	sx[0] = (int)(floor((m_bitmapOrigo[z] + c->GetLon() * m_pixelsPerLonDegree[z])*256.0f));
  	e = sin(c->GetLat() * (3.1415926535f/180.0f));

  	if(e > 0.9999)
    	e = 0.9999;

  	if(e < -0.9999)
    	e = -0.9999;

  	sy[0] = (int)(floor((m_bitmapOrigo[z] + 0.5f * log((1.0f + e) / (1.0f - e)) * m_negpixelsPerLonRadian[z])*256.0f));
}


/* convert from screen+scroll values to lon/lat */
void GGPXMap::FromMap(int sx,int sy,GPXCoord *c)
{
	double e;
	int z=GetZoom();

	c->SetLon(((double)sx - (m_bitmapOrigo[z]*256.0f)) / (m_pixelsPerLonDegree[z]*256.0f));
	e = ((double)sy - (m_bitmapOrigo[z]*256.0f)) / (m_negpixelsPerLonRadian[z]*256.0f);
	c->SetLat((2.0f * atan(exp(e)) - 3.1415926535f / 2.0f) / (3.1415926535f/180.0f));
}

/* draw tile at this position */
int GGPXMap::DrawTile(int tx,int ty)
{
	kGUIImage i;
	int serverid;
	int zoom=GetZoom();
//	static char xref[]={"qrts"};
	kGUIString gurl;
	kGUIString hstr;
	kGUIString fn;

	/* convert grid number to a filename */
	switch(m_type)
	{
	case MAPTYPE_GOOGLESAT:
	case MAPTYPE_GOOGLEHYBRID:
		/* this generates the URL and also returns the hstr which is used for the cached tile name */
		GenerateURL(TILESOURCE_SAT,&gurl,&hstr,&serverid,tx,ty,zoom);

		fn.Sprintf(GOOGLEMAPDIR "%s.jpg",hstr.GetString());
		i.SetFilename(fn.GetString());

		/* if image was not found on harddrive then try downloading it if user is online */
		if(i.IsValid()==false)
		{
			/* if file has size then it is not a valid image, so delete it */
			if(i.GetSize()>0)
			{
				/* not an image! */
				kGUI::FileDelete(fn.GetString());
				m_badurls.Add(gurl.GetString(),0);
				return(TILE_ERROR);	/* tile not found! */
			}

			if(g_isonline==true)
			{
				if(m_badurls.Find(gurl.GetString()))
					return(TILE_ERROR);		/* tile could not be loaded! */

				if(LoadTile(&gurl,&fn,tx,ty,serverid)==false)
					return(TILE_WAITING);
				if(gpx->GetMapAsync()==true)
					return(TILE_LOADING);		/* tile currently loading! */

				/* trigger a reload attempt */
				i.SetFilename(&fn);
			}
		}

		if(i.IsValid()==false)
		{
			i.Purge();
			return(TILE_ERROR);	/* tile not found! */
		}
		i.Draw(0,0,0);
		i.Purge();
		if(m_type==MAPTYPE_GOOGLEHYBRID)
		{
			fn.Sprintf(GOOGLEMAPDIR "x%d-%d-%d.png",17-zoom,tx,ty);
			i.SetFilename(fn.GetString());
			if(i.IsValid()==false)
			{
				GenerateURL(TILESOURCE_OVERLAY,&gurl,0,&serverid,tx,ty,zoom);

				/* if file has size then it is not a valid image, so delete it */
				if(i.GetSize()>0)
				{
					/* not an image! */
					kGUI::FileDelete(fn.GetString());
					m_badurls.Add(gurl.GetString(),0);
				}

				if(g_isonline==true)
				{
					if(!m_badurls.Find(gurl.GetString()))
					{
						if(LoadTile(&gurl,&fn,tx,ty,serverid|4)==false)
							return(TILE_WAITING);

						/* trigger a reload attempt */
						i.SetFilename(fn.GetString());
					}
				}
			}

			if(i.IsValid()==true)
				i.Draw(0,0,0);
			i.Purge();
		}
	break;
	case MAPTYPE_GOOGLEMAP:
		fn.Sprintf(GOOGLEMAPDIR "y%d-%d-%d.png",17-zoom,tx,ty);
		i.SetFilename(fn.GetString());
		if(i.IsValid()==false)
		{
			GenerateURL(TILESOURCE_MAP,&gurl,0,&serverid,tx,ty,zoom);

			/* if file has size then it is not a valid image, so delete it */
			if(i.GetSize()>0)
			{
				/* not an image! */
				kGUI::FileDelete(fn.GetString());
				m_badurls.Add(gurl.GetString(),0);
				return(TILE_ERROR);	/* tile not found! */
			}
			if(g_isonline==true)
			{
				if(m_badurls.Find(gurl.GetString()))
					return(TILE_ERROR);		/* tile could not be loaded! */

				if(LoadTile(&gurl,&fn,tx,ty,serverid|4)==false)
					return(TILE_WAITING);

				if(gpx->GetMapAsync()==true)
					return(TILE_LOADING);

				/* since file is now downloaded try again */
				i.SetFilename(fn.GetString());
			}
		}

		if(i.IsValid()==false)
		{
			i.Purge();
			return(TILE_ERROR);	/* not drawn */
		}
		i.Draw(0,0,0);
		i.Purge();
	break;
	case MAPTYPE_GOOGLETERRAIN:
		fn.Sprintf(GOOGLEMAPDIR "t%d-%d-%d.png",17-zoom,tx,ty);
		i.SetFilename(fn.GetString());
		if(i.IsValid()==false)
		{
			GenerateURL(TILESOURCE_TERRAIN,&gurl,0,&serverid,tx,ty,zoom);

			/* if file has size then it is not a valid image, so delete it */
			if(i.GetSize()>0)
			{
				/* not an image! */
				kGUI::FileDelete(fn.GetString());
				m_badurls.Add(gurl.GetString(),0);
				return(TILE_ERROR);	/* tile not found! */
			}
			if(g_isonline==true)
			{
				if(m_badurls.Find(gurl.GetString()))
					return(TILE_ERROR);		/* tile could not be loaded! */

				if(LoadTile(&gurl,&fn,tx,ty,serverid|4)==false)
					return(TILE_WAITING);

				if(gpx->GetMapAsync()==true)
					return(TILE_LOADING);

				/* since file is now downloaded try again */
				i.SetFilename(fn.GetString());
			}
		}

		if(i.IsValid()==false)
		{
			i.Purge();
			return(TILE_ERROR);	/* not drawn */
		}
		i.Draw(0,0,0);
		i.Purge();
	break;
	}
	return(TILE_OK);
}

/*************************************************************/

/* limit only 2 pending requests per serverid since trying to download too */
/* quickly triggers googles blocker */

bool GGPXMap::LoadTile(kGUIString *url,kGUIString *fn,int x,int y,int serverid)
{
	int i;
	GGPXMapTile *dle;
	int numactive;

	/* if the checkupdate is still running then wait for it to finish */
#if 0
#else
	if(m_checkthread.GetActive()==true)
		return(false);
#endif

	/* count total active downloads pending */
	numactive=0;
	for(i=0;i<MAXDLS;++i)
	{
		dle=&m_dles[i];
		if(dle->GetAsyncActive()==true)
			++numactive;
	}
	if(numactive>=gpx->GetMaxActiveDownloads())
		return(false);

	/* count number of active tiles using this serverid */
	numactive=0;
	for(i=0;i<MAXDLS;++i)
	{
		dle=&m_dles[i];
		if(dle->GetServerID()==serverid)
		{
			if(dle->GetAsyncActive()==true)
				++numactive;
		}
	}

	if(numactive>=2)
		return(false);

	/* check to see if this tile is already being loaded */
	for(i=0;i<MAXDLS;++i)
	{
		dle=&m_dles[i];
		if(dle->GetAsyncActive()==true)
		{
			if(!strcmp(dle->GetURL()->GetString(),url->GetString()))
			{
				if(gpx->GetMapAsync()==false)
					dle->WaitFinished();
				return(true);	/* already loading... */
			}
		}
	}

	/* find an available spot and start loading.... */
	for(i=0;i<MAXDLS;++i)
	{
		dle=&m_dles[i];
		if(dle->GetAsyncActive()==false)
		{
			dle->LoadTile(url,fn,x,y,serverid);
			return(true);
		}
	}
	/* no available slots for downloading */
	return(false);
}

void GGPXMapTile::LoadTile(kGUIString *url,kGUIString *fn,int x,int y,int serverid)
{
	m_serverid=serverid;
	m_x=x;
	m_y=y;
	m_dle.SetReferer("http://maps.google.com");
	m_dh.SetFilename(fn);
	DebugPrint("LoadTile start,x=%d,y=%d,url='%s'\n",m_x,m_y,url->GetString());

	m_dle.SetAllowCookies(false);
	if(gpx->GetMapAsync()==true)
		m_dle.AsyncDownLoad(&m_dh,url,this,CALLBACKNAME(TileLoaded));
	else
		m_dle.DownLoad(&m_dh,url);
}

void GGPXMapTile::TileLoaded(int result)
{
	/* if loading error then add url to bad list and make draw code */
	/* draw a broken shape if tile is in the bad list */

	if(result!=DOWNLOAD_OK)
		m_owner->AddBadURL(m_dle.GetURL());
	gpx->GridDirty(m_x,m_y);
}
