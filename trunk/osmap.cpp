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

#include "zlib/zlib.h"

enum
{
OSMTAG_NODE,
OSMTAG_WAY,
OSMTAG_ND,
OSMTAG_LON,
OSMTAG_LAT,
OSMTAG_TAG,
OSMTAG_K,
OSMTAG_V,
OSMTAG_ID,
OSMTAG_TIMESTAMP,
OSMTAG_USER,
OSMTAG_VISIBLE,
OSMTAG_PLACE_NAME,
OSMTAG_STOPSIGN,
OSMTAG_YIELDSIGN,
OSMTAG_NOLEFTTURN,
OSMTAG_NORIGHTTURN,
OSMTAG_IGNORE
};

#define SECTIONHEADERSIZE (5*sizeof(int)+4*sizeof(double))

enum
{
OSMRENDERTYPE_POLY,			/* poly or polyline */
OSMRENDERTYPE_OUTPOLY,		/* outlined poly or polyline */
OSMRENDERTYPE_POLYLINE,
OSMRENDERTYPE_BRIDGE,
OSMRENDERTYPE_TUNNEL,
OSMRENDERTYPE_TRACK,
OSMRENDERTYPE_BRIDGETRACK,
OSMRENDERTYPE_DASHED,
OSMRENDERTYPE_DOTTED,
OSMRENDERTYPE_POWER,
OSMRENDERTYPE_ICON,
OSMRENDERTYPE_HIDE
};

enum
{
OSMLINETYPE_LINE,
OSMLINETYPE_CREEK,
OSMLINETYPE_NARROWSTREET,
OSMLINETYPE_STREET,
OSMLINETYPE_RAMPS,
OSMLINETYPE_COLLECTOR,
OSMLINETYPE_HIGHWAY,
OSMLINETYPE_TOWER,
OSMLINETYPE_NUM};

/* thicknes stable for lines */

static const OSMRENDER_INFO osmrenderinfo[]={
	{1,{PLACE_CITY},			OSMRENDERTYPE_POLY,		0,						DrawColor(215,215,245),DrawColor(128,128,128)},
	{1,{PLACE_HAMLET},			OSMRENDERTYPE_POLY,		0,						DrawColor(217,217,247),DrawColor(128,128,128)},
	{1,{PLACE_COUNTY},			OSMRENDERTYPE_POLY,		0,						DrawColor(210,215,245),DrawColor(128,128,128)},
	{1,{PLACE_SUBURB},			OSMRENDERTYPE_POLY,		0,						DrawColor(220,220,250),DrawColor(128,128,128)},
	{1,{PLACE_TOWN},			OSMRENDERTYPE_POLY,		0,						DrawColor(235,230,220),DrawColor(128,128,128)},
	{1,{PLACE_VILLAGE},			OSMRENDERTYPE_POLY,		0,						DrawColor(230,230,255),DrawColor(128,128,128)},
	{1,{AMENITY_RESIDENTIAL},	OSMRENDERTYPE_POLY,		0,						DrawColor(235,230,255),DrawColor(128,128,128)},
	{1,{AMENITY_COMMERCIAL},	OSMRENDERTYPE_POLY,		0,						DrawColor(245,230,235),DrawColor(128,128,128)},
	{1,{AMENITY_INDUSTRIAL},	OSMRENDERTYPE_POLY,		0,						DrawColor(245,210,235),DrawColor(128,128,128)},
	{1,{AMENITY_COLLEGE},		OSMRENDERTYPE_POLY,		0,						DrawColor(205,220,235),DrawColor(128,128,128)},
	{1,{AMENITY_RETAIL},		OSMRENDERTYPE_POLY,		0,						DrawColor(240,220,205),DrawColor(128,128,128)},
	{1,{BORDER_COUNTY},			OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_LINE,		DrawColor(0,0,0),DrawColor(128,128,128)},
	{1,{BORDER_PROVINCE},		OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_LINE,		DrawColor(0,0,0),DrawColor(128,128,128)},
	{1,{BORDER_COUNTRY},		OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_LINE,		DrawColor(0,0,0),DrawColor(128,128,128)},
	{1,{BOUNDARY_ADMINISTRATIVE},OSMRENDERTYPE_POLYLINE, OSMLINETYPE_LINE,		DrawColor(0,0,0),DrawColor(128,128,128)},
	{1,{AEROWAY_APRON},			OSMRENDERTYPE_POLY,		0,						DrawColor(180,180,180),DrawColor(128,128,128)},
	{1,{RAILWAY_LAND},			OSMRENDERTYPE_POLY,		0,						DrawColor(170,170,180),DrawColor(128,128,128)},
	{1,{RAILWAY_YARD},			OSMRENDERTYPE_POLY,		0,						DrawColor(180,180,190),DrawColor(128,128,128)},
	{1,{AMENITY_FARM},			OSMRENDERTYPE_POLY,		0,						DrawColor(167,234,149),DrawColor(128,128,128)},
	{1,{NATURAL_BEACH},			OSMRENDERTYPE_POLY,		0,						DrawColor(225,234,255),DrawColor(128,128,128)},
	{1,{AMENITY_UNDERGROUNDPARKING},	OSMRENDERTYPE_DASHED,		0,			DrawColor(96,96,96),DrawColor(128,128,128)},
	{1,{AMENITY_MARINA},		OSMRENDERTYPE_DASHED,		0,					DrawColor(100,220,255),DrawColor(128,128,128)},

	{1,{LEISURE_PARK},			OSMRENDERTYPE_POLY,		0,						DrawColor(167,204,149),DrawColor(128,128,128)},
	{1,{SPORT_BASEBALL},		OSMRENDERTYPE_POLY,		0,						DrawColor(177,204,149),DrawColor(128,128,128)},
	{1,{SPORT_FOOTBALL},		OSMRENDERTYPE_POLY,		0,						DrawColor(177,204,159),DrawColor(128,128,128)},
	{1,{AMENITY_SWIMMINGPOOL},		OSMRENDERTYPE_POLY,		0,					DrawColor(153,179,255),DrawColor(128,128,128)},
	{1,{NATURAL_GRASS},			OSMRENDERTYPE_POLY,		0,						DrawColor(167,244,149),DrawColor(128,128,128)},
	{1,{NATURAL_FIELD},			OSMRENDERTYPE_POLY,		0,						DrawColor(177,234,159),DrawColor(128,128,128)},
	{1,{NATURAL_SCRUB},			OSMRENDERTYPE_POLY,		0,						DrawColor(197,244,169),DrawColor(128,128,128)},
	{1,{NATURAL_FOREST},		OSMRENDERTYPE_POLY,		0,						DrawColor(157,224,129),DrawColor(128,128,128)},
	{1,{NATURAL_LAND},			OSMRENDERTYPE_POLY,		0,						DrawColor(167,214,149),DrawColor(128,128,128)},
	{1,{NATURAL_RIVER},			OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_CREEK,		DrawColor(153,179,234),DrawColor(128,128,234)},
	{1,{NATURAL_STREAM},		OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_CREEK,		DrawColor(163,179,234),DrawColor(128,128,234)},
	{1,{NATURAL_WATER},			OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_CREEK,		DrawColor(153,179,224),DrawColor(128,128,234)},
	{1,{NATURAL_CANAL},			OSMRENDERTYPE_POLY,		0,						DrawColor(46,46,255),DrawColor(128,128,128)},
	{1,{NATURAL_LAKE},			OSMRENDERTYPE_POLY,		0,						DrawColor(66,66,255),DrawColor(128,128,128)},
	{1,{NATURAL_DRAIN},			OSMRENDERTYPE_POLY,		0,						DrawColor(76,87,255),DrawColor(128,128,128)},
	{1,{AMENITY_QUARRY},		OSMRENDERTYPE_POLY,		0,						DrawColor(184,134,11),DrawColor(128,128,128)},
	{1,{NATURAL_MARSH},			OSMRENDERTYPE_POLY,		0,						DrawColor(184,154,41),DrawColor(128,128,128)},
	{1,{AMENITY_LANDFILL},		OSMRENDERTYPE_POLY,		0,						DrawColor(175,124,11),DrawColor(128,128,128)},
	{1,{AEROWAY_AERODROME},		OSMRENDERTYPE_POLY,		0,						DrawColor(120,120,120),DrawColor(128,128,128)},
	{1,{AMENITY_AIRPORTTERMINAL},OSMRENDERTYPE_POLY,	0,						DrawColor(120,120,120),DrawColor(128,128,128)},
	{1,{AMENITY_ATTRACTION},	OSMRENDERTYPE_POLY,		0,						DrawColor(200,200,50),DrawColor(128,128,128)},
	{1,{AMENITY_WATERPARK},		OSMRENDERTYPE_POLY,		0,						DrawColor(200,200,255),DrawColor(128,128,128)},
	{1,{AMENITY_WATERTOWER},	OSMRENDERTYPE_POLY,		0,						DrawColor(50,205,50),DrawColor(128,128,128)},
	{1,{AMENITY_BUSSTATION},	OSMRENDERTYPE_POLY,		0,						DrawColor(230,230,225),DrawColor(128,128,128)},
	{1,{AMENITY_ZOO},			OSMRENDERTYPE_POLY,		0,						DrawColor(255,255,215),DrawColor(128,128,128)},

	{1,{LEISURE_RECREATIONAREA},OSMRENDERTYPE_POLY,		0,						DrawColor(167,210,149),DrawColor(128,128,128)},
	{1,{AMENITY_CAMPSITE},		OSMRENDERTYPE_POLY,		0,						DrawColor(207,210,89),DrawColor(128,128,128)},

	{1,{NATURAL_RIVERBANK},		OSMRENDERTYPE_POLYLINE,		OSMLINETYPE_LINE,	DrawColor(153,179,224),DrawColor(128,128,128)},
	{1,{NATURAL_COASTLINE},		OSMRENDERTYPE_POLYLINE,		OSMLINETYPE_LINE,	DrawColor(153,179,214),DrawColor(128,128,128)},
	{1,{NATURAL_RESERVOIR},		OSMRENDERTYPE_POLY,		0,						DrawColor(153,179,234),DrawColor(128,128,128)},
	{1,{AMENITY_GARDEN},		OSMRENDERTYPE_POLY,		0,						DrawColor(177,254,159),DrawColor(128,128,128)},
	{1,{AMENITY_GOLFCOURSE},	OSMRENDERTYPE_POLY,		0,						DrawColor(167,214,149),DrawColor(128,128,128)},
	{1,{AMENITY_UNIVERSITY},	OSMRENDERTYPE_POLY,		0,						DrawColor(230,230,255),DrawColor(128,128,128)},
	{1,{LEISURE_PITCH},			OSMRENDERTYPE_POLY,		0,						DrawColor(167,200,149),DrawColor(128,128,128)},
	{1,{SPORT_SOCCER},			OSMRENDERTYPE_POLY,		0,						DrawColor(167,200,149),DrawColor(128,128,128)},
	{1,{AMENITY_PLAYGROUND},	OSMRENDERTYPE_POLY,		0,						DrawColor(167,220,149),DrawColor(128,128,128)},
	{1,{SPORT_BASKETBALL},		OSMRENDERTYPE_POLY,		0,						DrawColor(187,200,189),DrawColor(128,128,128)},
	{1,{AMENITY_CEMETERY},		OSMRENDERTYPE_POLY,		0,						DrawColor(64,255,64),DrawColor(128,128,128)},

	{2,{RAILWAY_SUBWAY,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGETRACK,	OSMLINETYPE_STREET,		DrawColor(200,200,200),DrawColor(128,128,128)},
	{1,{RAILWAY_SUBWAY},		OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(200,200,200),DrawColor(128,128,128)},
	{1,{RAILWAY_HALT},			OSMRENDERTYPE_POLY,		0,						DrawColor(64,64,64),DrawColor(128,128,128)},
	{1,{AMENITY_PIPELINE},		OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_LINE,		DrawColor(0,0,0),DrawColor(128,128,128)},

	{1,{TUNNEL_TRUE},				OSMRENDERTYPE_TUNNEL,	OSMLINETYPE_STREET,		DrawColor(213,222,233),DrawColor(128,128,255)},
	{1,{ACCESS_PRIVATE},			OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(178,178,178),DrawColor(128,128,128)},
	{1,{AEROWAY_RUNWAY},			OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(150,150,150),DrawColor(128,128,128)},
	{1,{AEROWAY_TAXIWAY},			OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(150,150,150),DrawColor(128,128,128)},
	{1,{HIGHWAY_SERVICE},			OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(245,245,245),DrawColor(128,128,128)},
	{1,{HIGHWAY_PATH},				OSMRENDERTYPE_DOTTED,	0,						DrawColor(255,128,128),DrawColor(128,128,128)},
	{1,{HIGHWAY_PEDESTRIAN},		OSMRENDERTYPE_DOTTED,	0,						DrawColor(255,128,128),DrawColor(128,128,128)},
	{1,{HIGHWAY_STEPS},				OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(198,198,188),DrawColor(128,128,128)},
	{1,{HIGHWAY_FOOTWAY},			OSMRENDERTYPE_DOTTED,	0,						DrawColor(255,128,128),DrawColor(128,128,128)},
	{1,{HIGHWAY_BRIDLEWAY},			OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(198,208,198),DrawColor(128,128,128)},
	{1,{HIGHWAY_TERTIARY},			OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{1,{HIGHWAY_TRACK},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(234,234,234),DrawColor(128,128,128)},
	{1,{HIGHWAY_ROAD},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{2,{HIGHWAY_ROAD,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{1,{HIGHWAY_RESIDENTIAL},		OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{1,{HIGHWAY_MINOR}		,		OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{1,{HIGHWAY_UNCLASSIFIED},		OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(224,224,224),DrawColor(128,128,128)},
	{1,{HIGHWAY_CROSSING},			OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,0,0),DrawColor(128,128,128)},
	{1,{JUNCTION_ROUNDABOUT},		OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},

	/* base layer */
	{1,{HIGHWAY_SECONDARY},					OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{2,{HIGHWAY_SECONDARY,BRIDGE_TRUE},		OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{1,{HIGHWAY_SECONDARYLINK},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{2,{HIGHWAY_SECONDARYLINK,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{1,{HIGHWAY_PRIMARY},					OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{2,{HIGHWAY_PRIMARY,BRIDGE_TRUE},		OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{1,{HIGHWAY_PRIMARYLINK},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{2,{HIGHWAY_PRIMARYLINK,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{1,{HIGHWAY_TRUNK},						OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(128,128,128)},
	{2,{HIGHWAY_TRUNK,BRIDGE_TRUE},			OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(64,64,64)},
	{1,{HIGHWAY_TRUNK_LINK},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(128,128,128)},
	{2,{HIGHWAY_TRUNK_LINK,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(64,64,64)},
	{1,{HIGHWAY_MOTORWAY},					OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_HIGHWAY,	DrawColor(242,191,36),DrawColor(128,128,128)},
	{2,{HIGHWAY_MOTORWAY,BRIDGE_TRUE},		OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_HIGHWAY,	DrawColor(242,191,36),DrawColor(64,64,64)},
	{1,{HIGHWAY_MOTORWAY_LINK},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_RAMPS,		DrawColor(242,181,36),DrawColor(128,128,128)},
	{2,{HIGHWAY_MOTORWAY_LINK,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_RAMPS,		DrawColor(242,181,36),DrawColor(64,64,64)},

	{1,{HIGHWAY_CYCLEWAY},			OSMRENDERTYPE_DOTTED,	0,						DrawColor(32,32,255),DrawColor(128,128,128)},
	{1,{CYCLEWAY_TRACK},			OSMRENDERTYPE_DOTTED,	0,						DrawColor(64,64,255),DrawColor(128,128,128)},
	{1,{CYCLEWAY_LANE},				OSMRENDERTYPE_DOTTED,	0,						DrawColor(96,96,255),DrawColor(128,128,128)},

	/* railway layer goes after base street layer */
	{2,{RAILWAY_ABANDONED,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGETRACK,	OSMLINETYPE_STREET,	DrawColor(180,180,180),DrawColor(64,64,64)},
	{1,{RAILWAY_ABANDONED},				OSMRENDERTYPE_TRACK,	0,						DrawColor(96,96,96),DrawColor(128,128,128)},
	{2,{RAILWAY_LIGHTRAIL,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGETRACK,	OSMLINETYPE_STREET,	DrawColor(180,180,180),DrawColor(64,64,64)},
	{1,{RAILWAY_LIGHTRAIL},				OSMRENDERTYPE_TRACK,	0,						DrawColor(64,64,96),DrawColor(128,128,128)},
	{2,{RAILWAY_TRAM,BRIDGE_TRUE},		OSMRENDERTYPE_BRIDGETRACK,	OSMLINETYPE_STREET,	DrawColor(180,180,180),DrawColor(64,64,64)},
	{1,{RAILWAY_TRAM},					OSMRENDERTYPE_TRACK,	0,						DrawColor(96,64,96),DrawColor(128,128,128)},
	{2,{RAILWAY_RAIL,BRIDGE_TRUE},		OSMRENDERTYPE_BRIDGETRACK,	OSMLINETYPE_STREET,	DrawColor(180,180,180),DrawColor(64,64,64)},
	{1,{RAILWAY_RAIL},					OSMRENDERTYPE_TRACK,	0,						DrawColor(64,64,64),DrawColor(128,128,128)},
	{2,{RAILWAY_SPUR,BRIDGE_TRUE},		OSMRENDERTYPE_BRIDGETRACK,	OSMLINETYPE_STREET,	DrawColor(180,180,180),DrawColor(64,64,64)},
	{1,{RAILWAY_SPUR},					OSMRENDERTYPE_TRACK,	0,						DrawColor(64,64,64),DrawColor(128,128,128)},
	{2,{RAILWAY_MONORAIL,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGETRACK,	OSMLINETYPE_STREET,	DrawColor(180,180,180),DrawColor(64,64,64)},
	{1,{RAILWAY_MONORAIL},				OSMRENDERTYPE_TRACK,	0,						DrawColor(64,64,64),DrawColor(128,128,128)},
	{2,{RAILWAY_UNKNOWN,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGETRACK,	OSMLINETYPE_STREET,	DrawColor(180,180,180),DrawColor(64,64,64)},
	{1,{RAILWAY_UNKNOWN},				OSMRENDERTYPE_TRACK,	0,						DrawColor(64,64,64),DrawColor(128,128,128)},

	/* layer 1 */
	{2,{HIGHWAY_SECONDARY,LAYER_1},					OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{3,{HIGHWAY_SECONDARY,LAYER_1,BRIDGE_TRUE},		OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{2,{HIGHWAY_SECONDARYLINK,LAYER_1},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{3,{HIGHWAY_SECONDARYLINK,LAYER_1,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{2,{HIGHWAY_PRIMARY,LAYER_1},					OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{3,{HIGHWAY_PRIMARY,LAYER_1,BRIDGE_TRUE},		OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{2,{HIGHWAY_PRIMARYLINK,LAYER_1},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{3,{HIGHWAY_PRIMARYLINK,LAYER_1,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{2,{HIGHWAY_TRUNK,LAYER_1},						OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(128,128,128)},
	{3,{HIGHWAY_TRUNK,LAYER_1,BRIDGE_TRUE},			OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(64,64,64)},
	{2,{HIGHWAY_TRUNK_LINK,LAYER_1},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(128,128,128)},
	{3,{HIGHWAY_TRUNK_LINK,LAYER_1,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(64,64,64)},
	{2,{HIGHWAY_MOTORWAY,LAYER_1},					OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_HIGHWAY,	DrawColor(242,191,36),DrawColor(128,128,128)},
	{3,{HIGHWAY_MOTORWAY,LAYER_1,BRIDGE_TRUE},		OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_HIGHWAY,	DrawColor(242,191,36),DrawColor(64,64,64)},
	{2,{HIGHWAY_MOTORWAY_LINK,LAYER_1},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_RAMPS,		DrawColor(242,181,36),DrawColor(128,128,128)},
	{3,{HIGHWAY_MOTORWAY_LINK,LAYER_1,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_RAMPS,		DrawColor(242,191,36),DrawColor(64,64,64)},

	/* layer 2 */
	{2,{HIGHWAY_SECONDARY,LAYER_2},					OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{3,{HIGHWAY_SECONDARY,LAYER_2,BRIDGE_TRUE},		OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{2,{HIGHWAY_SECONDARYLINK,LAYER_2},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{3,{HIGHWAY_SECONDARYLINK,LAYER_2,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{2,{HIGHWAY_PRIMARY,LAYER_2},					OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{3,{HIGHWAY_PRIMARY,LAYER_2,BRIDGE_TRUE},		OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{2,{HIGHWAY_PRIMARYLINK,LAYER_2},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{3,{HIGHWAY_PRIMARYLINK,LAYER_2,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{2,{HIGHWAY_TRUNK,LAYER_2},						OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(128,128,128)},
	{3,{HIGHWAY_TRUNK,LAYER_2,BRIDGE_TRUE},			OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(64,64,64)},
	{2,{HIGHWAY_TRUNK_LINK,LAYER_2},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(128,128,128)},
	{3,{HIGHWAY_TRUNK_LINK,LAYER_2,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(64,64,64)},
	{2,{HIGHWAY_MOTORWAY,LAYER_2},					OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_HIGHWAY,	DrawColor(242,191,36),DrawColor(128,128,128)},
	{3,{HIGHWAY_MOTORWAY,LAYER_2,BRIDGE_TRUE},		OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_HIGHWAY,	DrawColor(242,191,36),DrawColor(64,64,64)},
	{2,{HIGHWAY_MOTORWAY_LINK,LAYER_2},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_RAMPS,		DrawColor(242,181,36),DrawColor(128,128,128)},
	{3,{HIGHWAY_MOTORWAY_LINK,LAYER_2,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_RAMPS,		DrawColor(242,191,36),DrawColor(64,64,64)},

	/* layer 3 */
	{2,{HIGHWAY_SECONDARY,LAYER_3},					OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{3,{HIGHWAY_SECONDARY,LAYER_3,BRIDGE_TRUE},		OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{2,{HIGHWAY_SECONDARYLINK,LAYER_3},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{3,{HIGHWAY_SECONDARYLINK,LAYER_3,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{2,{HIGHWAY_PRIMARY,LAYER_3},					OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{3,{HIGHWAY_PRIMARY,LAYER_3,BRIDGE_TRUE},		OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{2,{HIGHWAY_PRIMARYLINK,LAYER_3},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{3,{HIGHWAY_PRIMARYLINK,LAYER_3,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{2,{HIGHWAY_TRUNK,LAYER_3},						OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(128,128,128)},
	{3,{HIGHWAY_TRUNK,LAYER_3,BRIDGE_TRUE},			OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(64,64,64)},
	{2,{HIGHWAY_TRUNK_LINK,LAYER_3},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(128,128,128)},
	{3,{HIGHWAY_TRUNK_LINK,LAYER_3,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(64,64,64)},
	{2,{HIGHWAY_MOTORWAY,LAYER_3},					OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_HIGHWAY,	DrawColor(242,191,36),DrawColor(128,128,128)},
	{3,{HIGHWAY_MOTORWAY,LAYER_3,BRIDGE_TRUE},		OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_HIGHWAY,	DrawColor(242,191,36),DrawColor(64,64,64)},
	{2,{HIGHWAY_MOTORWAY_LINK,LAYER_3},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_RAMPS,		DrawColor(242,181,36),DrawColor(128,128,128)},
	{3,{HIGHWAY_MOTORWAY_LINK,LAYER_3,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_RAMPS,		DrawColor(242,191,36),DrawColor(64,64,64)},

	/* layer 4 */
	{2,{HIGHWAY_SECONDARY,LAYER_4},					OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{3,{HIGHWAY_SECONDARY,LAYER_4,BRIDGE_TRUE},		OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{2,{HIGHWAY_SECONDARYLINK,LAYER_4},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{3,{HIGHWAY_SECONDARYLINK,LAYER_4,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{2,{HIGHWAY_PRIMARY,LAYER_4},					OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{3,{HIGHWAY_PRIMARY,LAYER_4,BRIDGE_TRUE},		OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{2,{HIGHWAY_PRIMARYLINK,LAYER_4},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{3,{HIGHWAY_PRIMARYLINK,LAYER_4,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{2,{HIGHWAY_TRUNK,LAYER_4},						OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(128,128,128)},
	{3,{HIGHWAY_TRUNK,LAYER_4,BRIDGE_TRUE},			OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(64,64,64)},
	{2,{HIGHWAY_TRUNK_LINK,LAYER_4},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(128,128,128)},
	{3,{HIGHWAY_TRUNK_LINK,LAYER_4,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(64,64,64)},
	{2,{HIGHWAY_MOTORWAY,LAYER_4},					OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_HIGHWAY,	DrawColor(242,191,36),DrawColor(128,128,128)},
	{3,{HIGHWAY_MOTORWAY,LAYER_4,BRIDGE_TRUE},		OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_HIGHWAY,	DrawColor(242,191,36),DrawColor(64,64,64)},
	{2,{HIGHWAY_MOTORWAY_LINK,LAYER_4},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_RAMPS,		DrawColor(242,181,36),DrawColor(128,128,128)},
	{3,{HIGHWAY_MOTORWAY_LINK,LAYER_4,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_RAMPS,		DrawColor(242,191,36),DrawColor(64,64,64)},

	/* layer 5 */
	{2,{HIGHWAY_SECONDARY,LAYER_5},					OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{3,{HIGHWAY_SECONDARY,LAYER_5,BRIDGE_TRUE},		OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{2,{HIGHWAY_SECONDARYLINK,LAYER_5},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{3,{HIGHWAY_SECONDARYLINK,LAYER_5,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{2,{HIGHWAY_PRIMARY,LAYER_5},					OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{3,{HIGHWAY_PRIMARY,LAYER_5,BRIDGE_TRUE},		OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{2,{HIGHWAY_PRIMARYLINK,LAYER_5},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(128,128,128)},
	{3,{HIGHWAY_PRIMARYLINK,LAYER_5,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_STREET,		DrawColor(255,255,255),DrawColor(64,64,64)},
	{2,{HIGHWAY_TRUNK,LAYER_5},						OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(128,128,128)},
	{3,{HIGHWAY_TRUNK,LAYER_5,BRIDGE_TRUE},			OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(64,64,64)},
	{2,{HIGHWAY_TRUNK_LINK,LAYER_5},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(128,128,128)},
	{3,{HIGHWAY_TRUNK_LINK,LAYER_5,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112),DrawColor(64,64,64)},
	{2,{HIGHWAY_MOTORWAY,LAYER_5},					OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_HIGHWAY,	DrawColor(242,191,36),DrawColor(128,128,128)},
	{3,{HIGHWAY_MOTORWAY,LAYER_5,BRIDGE_TRUE},		OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_HIGHWAY,	DrawColor(242,191,36),DrawColor(64,64,64)},
	{2,{HIGHWAY_MOTORWAY_LINK,LAYER_5},				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_RAMPS,		DrawColor(242,181,36),DrawColor(128,128,128)},
	{3,{HIGHWAY_MOTORWAY_LINK,LAYER_5,BRIDGE_TRUE},	OSMRENDERTYPE_BRIDGE,	OSMLINETYPE_RAMPS,		DrawColor(242,191,36),DrawColor(64,64,64)},

	{1,{SURFACE_PAVED},			OSMRENDERTYPE_OUTPOLY,		0,	DrawColor(32,32,32),DrawColor(128,128,128)},
	{1,{SURFACE_GRAVEL},		OSMRENDERTYPE_OUTPOLY,		0,	DrawColor(139,109,79),DrawColor(128,128,128)},
	{1,{SURFACE_DIRT},			OSMRENDERTYPE_OUTPOLY,		0,	DrawColor(139,69,19),DrawColor(128,128,128)},
	{1,{HIGHWAY_UNPAVED},		OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(139,79,29),DrawColor(128,128,128)},
	{1,{HIGHWAY_BRIDGE},		OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(224,224,255),DrawColor(0,0,0)},
	{1,{BRIDGE_TRUE},			OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(224,224,255),DrawColor(128,128,128)},

	{1,{AMENITY_PARKING},			OSMRENDERTYPE_OUTPOLY,		0,					DrawColor(96,96,96),DrawColor(128,128,128)},
	{1,{AMENITY_BICYCLEPARKING},	OSMRENDERTYPE_OUTPOLY,		0,					DrawColor(200,200,200),DrawColor(128,128,128)},
	{1,{AMENITY_SERVICEYARD},		OSMRENDERTYPE_OUTPOLY,		0,					DrawColor(148,148,148),DrawColor(128,128,128)},

	/* buildings */
	{1,{AMENITY_AMBULANCESTATION},	OSMRENDERTYPE_OUTPOLY,		0,					DrawColor(255,32,32),DrawColor(128,128,128)},
	{1,{AMENITY_HOSPITAL},		OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(255,128,96),DrawColor(128,128,128)},
	{1,{AMENITY_FIRESTATION},	OSMRENDERTYPE_OUTPOLY,		0,					DrawColor(255,64,32),DrawColor(128,128,128)},
	{1,{AMENITY_POLICE},		OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(64,64,64),DrawColor(128,128,128)},
	{1,{AMENITY_BANK},			OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(64,96,64),DrawColor(128,128,128)},
	{1,{AMENITY_PUBLICBUILDING},OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(96,128,96),DrawColor(128,128,128)},
	{1,{AMENITY_BUILDING},		OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(96,64,64),DrawColor(128,128,128)},
	{1,{AMENITY_TOWER},			OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(96,96,64),DrawColor(128,128,128)},
	{1,{BUILDING_TRUE},			OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(96,64,64),DrawColor(128,128,128)},
	{1,{AMENITY_LIBRARY},		OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(205,225,225),DrawColor(128,128,128)},
	{1,{AMENITY_TOWNHALL},		OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(225,225,205),DrawColor(128,128,128)},
	{1,{AMENITY_STADIUM},		OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(225,225,225),DrawColor(128,128,128)},
	{1,{AMENITY_SCHOOL},		OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(255,192,192),DrawColor(128,128,128)},
	{1,{AMENITY_SHOPPING},		OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(150,150,150),DrawColor(128,128,128)},
	{1,{SPORT_TENNIS},			OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(220,255,220),DrawColor(255,255,255)},
	{1,{AMENITY_HOCKEYRINK},	OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(230,255,255),DrawColor(128,128,128)},
	{1,{AMENITY_SPORTSTRACK},	OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(238,221,130),DrawColor(128,128,128)},
	{1,{POWER_SUBSTATION},		OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(140,140,140),DrawColor(128,128,128)},
	{1,{POWER_GENERATOR},		OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(150,150,150),DrawColor(128,128,128)},
	{1,{AMENITY_SUPERMARKET},	OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(220,180,230),DrawColor(128,128,128)},
	{1,{AMENITY_DAM},			OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(245,245,245),DrawColor(128,128,128)},
	{1,{AMENITY_HANGAR},		OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(205,245,245),DrawColor(128,128,128)},
	{1,{AMENITY_CHURCH},		OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(220,245,255),DrawColor(128,128,128)},
	{1,{AMENITY_PRISON},		OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(255,245,200),DrawColor(128,128,128)},
	{1,{AMENITY_RESTRAUNT},		OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(235,245,215),DrawColor(128,128,128)},
	{1,{AMENITY_FASTFOOD},		OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(235,245,225),DrawColor(128,128,128)},
	{1,{AMENITY_MOTEL},			OSMRENDERTYPE_OUTPOLY,		0,						DrawColor(215,235,225),DrawColor(128,128,128)},
	{1,{AMENITY_COMMUNITYCENTER},	OSMRENDERTYPE_OUTPOLY,	0,						DrawColor(255,255,220),DrawColor(128,128,128)},
	{1,{AMENITY_RECREATIONCENTER},	OSMRENDERTYPE_OUTPOLY,	0,						DrawColor(255,255,190),DrawColor(128,128,128)},
	{1,{AMENITY_MUSEUM},		OSMRENDERTYPE_OUTPOLY,	0,								DrawColor(255,255,192),DrawColor(128,128,128)},
	{1,{AMENITY_POSTOFFICE},	OSMRENDERTYPE_OUTPOLY,	0,							DrawColor(197,217,237),DrawColor(128,128,128)},
	{1,{AMENITY_PHARMACY},		OSMRENDERTYPE_OUTPOLY,	0,							DrawColor(207,197,237),DrawColor(128,128,128)},
	{1,{AMENITY_FUEL},			OSMRENDERTYPE_OUTPOLY,	0,							DrawColor(207,227,207),DrawColor(128,128,128)},
	{1,{AMENITY_HOTEL},			OSMRENDERTYPE_OUTPOLY,	0,							DrawColor(207,200,225),DrawColor(128,128,128)},
	{1,{RAILWAY_STATION},		OSMRENDERTYPE_OUTPOLY,	0,							DrawColor(232,210,225),DrawColor(128,128,128)},
	{1,{AMENITY_RECYCLING},		OSMRENDERTYPE_OUTPOLY,	0,							DrawColor(232,200,225),DrawColor(128,128,128)},

	/* small items */
	{1,{AMENITY_TOILETS},		OSMRENDERTYPE_OUTPOLY,	0,							DrawColor(255,255,128),DrawColor(128,128,128)},
	{1,{AMENITY_BENCH},			OSMRENDERTYPE_OUTPOLY,	0,							DrawColor(194,144,13),DrawColor(128,128,128)},
	{1,{AMENITY_FOUNTAIN},		OSMRENDERTYPE_OUTPOLY,	0,							DrawColor(205,92,92),DrawColor(128,128,128)},
	{1,{AMENITY_MONUMENT},		OSMRENDERTYPE_OUTPOLY,	0,							DrawColor(221,160,221),DrawColor(128,128,128)},
	{1,{HIGHWAY_GATE},			OSMRENDERTYPE_OUTPOLY,	0,							DrawColor(221,221,221),DrawColor(128,128,128)},

	{1,{FOOT_TRUE},				OSMRENDERTYPE_DOTTED,		0,					DrawColor(255,128,128),DrawColor(128,128,128)},
	{1,{AMENITY_PIER},			OSMRENDERTYPE_OUTPOLY,		0,					DrawColor(218,165,32),DrawColor(128,128,128)},
	{1,{AMENITY_DOCK},			OSMRENDERTYPE_OUTPOLY,		0,					DrawColor(228,185,42),DrawColor(128,128,128)},

	/* overhead items */
	{1,{AERIALWAY_UNKNOWN},		OSMRENDERTYPE_DASHED,	0,						DrawColor(0,0,0),DrawColor(128,128,128)},
	{1,{AMENITY_CHAIRLIFT},		OSMRENDERTYPE_DASHED,	0,						DrawColor(0,0,0),DrawColor(128,128,128)},
	{1,{ROUTE_FERRY},			OSMRENDERTYPE_DASHED,	0,						DrawColor(64,64,80),DrawColor(128,128,128)},
	{1,{ROUTE_SKI},				OSMRENDERTYPE_DASHED,	0,						DrawColor(64,64,80),DrawColor(128,128,128)},
	{1,{POWER_LINE},			OSMRENDERTYPE_POWER,	0,						DrawColor(64,80,64),DrawColor(128,128,128)},

	/* hidden objects, don't bother rendering these */
	{1,{HIGHWAY_CONSTRUCTION},		OSMRENDERTYPE_HIDE,		0,						DrawColor(128,128,128),DrawColor(128,128,128)},
	{1,{HIGHWAY_PROPOSED},			OSMRENDERTYPE_HIDE,		0,						DrawColor(128,128,128),DrawColor(128,128,128)},
	{1,{RAILWAY_PROPOSED},			OSMRENDERTYPE_HIDE,		0,						DrawColor(128,128,128),DrawColor(128,128,128)},
	{1,{PROPOSED_RESIDENTIAL},		OSMRENDERTYPE_HIDE,		0,						DrawColor(128,128,128),DrawColor(128,128,128)},
	{1,{PROPOSED_PRIMARYLINK},		OSMRENDERTYPE_HIDE,		0,						DrawColor(128,128,128),DrawColor(128,128,128)},
	{1,{CONSTRUCTION_RESIDENTIAL},	OSMRENDERTYPE_HIDE,		0,						DrawColor(128,128,128),DrawColor(128,128,128)},
	{1,{CONSTRUCTION_PRIMARY},		OSMRENDERTYPE_HIDE,		0,						DrawColor(128,128,128),DrawColor(128,128,128)},

	//undefined type is last entry in table!
	{0,{},						OSMRENDERTYPE_POLY,		0,						DrawColor(255,0,255),DrawColor(255,0,255)}};

#define NUMRENDERTYPES (sizeof(osmrenderinfo)/sizeof(osmrenderinfo[0]))
#define UNDEFINEDRENDERTYPE (NUMRENDERTYPES-1)

static const float osmthickinfo[OSMLINETYPE_NUM][MAXMSZOOM]={
	//0    1    2    3    4    5    6    7    8    0   10    11    12    13    14    15    16    17    18    19
	{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},		/* line ( topo etc. )*/
	{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,1.0f, 1.0f, 1.0f, 1.0f, 2.5f, 5.0f, 7.0f, 8.0f, 9.0f,10.0f},		/* creek */
	{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.4f,0.5f, 0.6f, 0.7f, 1.0f, 2.0f, 5.0f, 8.0f, 7.0f,10.0f,11.0f},		/* narrow street */
	{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,1.0f,1.0f, 1.0f, 1.0f, 2.0f, 4.5f, 9.0f,12.0f,13.0f,14.0f,15.0f},		/* side street */
	{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,1.0f,2.0f,2.0f, 2.5f, 3.0f, 4.0f, 6.0f,11.0f,13.0f,14.0f,15.0f,16.0f},		/* ramps */
	{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,1.0f,2.0f,2.5f,2.0f, 4.0f, 6.0f, 9.0f,10.0f,13.0f,14.0f,15.0f,16.0f,17.0f},		/* artery */
	{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,2.0f,3.0f,4.0f,5.0f, 7.0f, 9.0f,12.0f,15.0f,16.0f,17.0f,18.0f,19.0f,20.0f},		/* highway */
	{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f, 0.0f, 0.0f, 2.0f, 2.3f, 2.6f, 3.0f,3.33f,3.66f, 4.0f}};	/* powerline tower size */

kGUIFPoint2 OSMMap::m_ppoints[MAXPP];
kGUIFPoint2 OSMMap::m_ppoints2[MAXPP];

/* openstreetmap.com format map renderer */

static const char osmver[]={"0002"};

OSMSection::OSMSection()
{
	m_node=0;
	m_way=0;
	/* pointer lists for way->nodes and strings are stored here */
	m_heap.SetBlockSize(65536);
}

OSMSection::~OSMSection()
{
	if(m_node)
		delete []m_node;
	if(m_way)
		delete []m_way;
}

void OSMSection::LoadHeader(DataHandle *dh)
{
	double c;

	dh->Read(&m_numnodes,(unsigned long)4);
	dh->Read(&m_numways,(unsigned long)4);

	/* minlat / maxlat / minlon / maxlon */
	dh->Read(&c,(unsigned long)sizeof(double));
	m_se.SetLat(c);
	dh->Read(&c,(unsigned long)sizeof(double));
	m_nw.SetLat(c);
	dh->Read(&c,(unsigned long)sizeof(double));
	m_nw.SetLon(c);
	dh->Read(&c,(unsigned long)sizeof(double));
	m_se.SetLon(c);

	/* used to convert coords from doubles to unsigned 16 bit integers */
	m_latscale=(m_nw.GetLat()-m_se.GetLat())/65535.0f;
	m_lonscale=(m_se.GetLon()-m_nw.GetLon())/65535.0f;

	dh->Read(&m_offset,(unsigned long)4);
	dh->Read(&m_packedlength,(unsigned long)4);
	dh->Read(&m_unpackedlength,(unsigned long)4);
	m_loaded=false;
}

void OSMSection::Load(DataHandle *dh)
{
	unsigned int i;
	unsigned int j;
	OSMSECNODE_DEF *n;
	OSMSECWAY_DEF *w;
	unsigned short s2;
	GPXCoord *nlist;
    z_stream d_stream; /* decompression stream */
	Array<unsigned char>packed;
	Array<unsigned char>unpacked;
	DataHandle udh;
	int zret;
	bool hasname;
	int slen;
	char c;

	dh->Seek(m_offset);

	/* load packed buffer, then uncompress */
	packed.Init(m_packedlength,-1);
	dh->Read((void *)packed.GetArrayPtr(),(unsigned long)m_packedlength);
	unpacked.Init(m_unpackedlength,0);

	d_stream.zalloc = (alloc_func)0;
	d_stream.zfree = (free_func)0;
	d_stream.opaque = (voidpf)0;

	d_stream.next_in  = packed.GetArrayPtr();
	d_stream.avail_in= m_packedlength;
	d_stream.next_out = unpacked.GetArrayPtr();
	d_stream.avail_out= m_unpackedlength;

	inflateInit(&d_stream);
	zret=inflate(&d_stream, Z_FINISH);
    inflateEnd(&d_stream);

	m_loaded=true;

	udh.SetMemory(unpacked.GetArrayPtr(),m_unpackedlength);
	udh.Open();
	dh=&udh;

	/* load nodes */
	if(m_numnodes)
	{
		n=m_node=new OSMSECNODE_DEF[m_numnodes];
		for(i=0;i<m_numnodes;++i)
		{
			dh->Read((void *)&s2,(unsigned long)sizeof(s2));
			n->c.SetLat(s2*m_latscale+m_se.GetLat());
			dh->Read((void *)&s2,(unsigned long)sizeof(s2));
			n->c.SetLon(s2*m_lonscale+m_nw.GetLon());
			dh->Read((void *)&s2,(unsigned long)sizeof(s2));
			hasname=(s2&32768)!=0;
			s2&=0x7fff;
			n->m_render=&osmrenderinfo[s2];
			if(hasname)
			{
				slen=0;
				do
				{
					dh->Read((void *)&c,(unsigned long)1);
					packed.SetEntry(slen++,c);
				}while(c);
				n->m_name=(char *)m_heap.Alloc(slen);
				memcpy(n->m_name,packed.GetArrayPtr(),slen);
			}
			else
				n->m_name=0;
	
			++n;
		}
	}
	else
		m_node=0;

	/* load ways (polygons) */
	w=m_way=new OSMSECWAY_DEF[m_numways];
	for(i=0;i<m_numways;++i)
	{
		dh->Read((void *)&s2,(unsigned long)sizeof(s2));
		hasname=(s2&32768)!=0;
		w->m_closed=(s2&16384)!=0;
		s2&=0x3fff;
		w->m_render=&osmrenderinfo[s2];

		dh->Read((void *)&s2,(unsigned long)sizeof(s2));
		w->m_numcoords=s2;

		/* allocate array of node pointers on heap */
		w->m_coords=nlist=(GPXCoord *)m_heap.Alloc(w->m_numcoords*sizeof(GPXCoord));
		for(j=0;j<w->m_numcoords;++j)
		{
			dh->Read((void *)&s2,(unsigned long)sizeof(s2));
			nlist->SetLat(s2*m_latscale+m_se.GetLat());
			dh->Read((void *)&s2,(unsigned long)sizeof(s2));
			nlist->SetLon(s2*m_lonscale+m_nw.GetLon());

			if(!j)
			{
				w->m_min.Set(nlist->GetLat(),nlist->GetLon());
				w->m_max.Set(nlist->GetLat(),nlist->GetLon());
			}
			else
			{
				w->m_min.SetLat(valmin(w->m_min.GetLat(),nlist->GetLat()));
				w->m_min.SetLon(valmin(w->m_min.GetLon(),nlist->GetLon()));

				w->m_max.SetLat(valmax(w->m_max.GetLat(),nlist->GetLat()));
				w->m_max.SetLon(valmax(w->m_max.GetLon(),nlist->GetLon()));
			}
			++nlist;
		}
		if(hasname)
		{
			slen=0;
			do
			{
				dh->Read((void *)&c,(unsigned long)1);
				packed.SetEntry(slen++,c);
			}while(c);
			w->m_name=(char *)m_heap.Alloc(slen);
			memcpy(w->m_name,packed.GetArrayPtr(),slen);
		}
		else
			w->m_name=0;
		++w;
	}
	udh.Close();
}

/* new binary format */
OSMMap::OSMMap(const char *fn)
{
	DataHandle dh;
	unsigned int i;
	unsigned char block[4];
	OSMSection *section;
	double c;
	int nt;

	m_t.SetEncoding(ENCODING_UTF8);
	m_lc.Init(256,256);
	//save for loading sections later!
	m_filename.SetString(fn);

	/* used for 2 pass rendering for fatlines */
	m_drawheap.Alloc(65536);
	m_drawways.Init(1024,256);

	SetZoomLevels(MINOSMZOOM,MAXOSMZOOM);
	SetTileSize(256,256);
	for(i=0;i<MAXOSMZOOM;++i)
	{
		c=(double)(1<<i);
		m_pixelsPerLonDegree[i]=c/360.0f;
  		m_negpixelsPerLonRadian[i] = -(c / (2.0f*3.1415926535f));
		m_bitmapOrigo[i]=c/2.0f;
		nt = 1<<i;
		SetSize(i,nt*256,nt*256);
	}

	dh.SetFilename(fn);
	if(dh.Open()==false)
	{
isbad:	m_bad=true;
		return;
	}
	dh.Read((void *)&block,(unsigned long)4);
	if(!(block[0]=='o' && block[1]=='s' && block[2]=='m' && block[3]=='b'))
	{
		dh.Close();
		goto isbad;
	}
	dh.Read((void *)&block,(unsigned long)4);
	if(!(block[0]==osmver[0] && block[1]==osmver[1] && block[2]==osmver[2] && block[3]==osmver[3]))
	{
		dh.Close();
		goto isbad;
	}

	/* num sections */
	dh.Read((void *)&m_numsections,(unsigned long)4);
	/* todo, change to simple array, no need for class array */
	m_sections.Init(m_numsections,0);

	for(i=0;i<m_numsections;++i)
	{
		section=m_sections.GetEntryPtr(i);
		section->LoadHeader(&dh);
	}
	dh.Close();
}

#if DRAWDEBUG
void OSMMap::DebugDrawType(OSMSECWAY_DEF *w,kGUICorners *wc)
{
	//debugging, draw selected render type in the center of the object
	int fs,cx,lh,lw,y;
	unsigned int pw;

	m_t.Sprintf("%d",((int)w->m_render-(int)&osmrenderinfo)/sizeof(osmrenderinfo[0]));
	fs=9;
	pw=wc->rx-wc->lx;
	/* expand font size until it doesn't fit anymore or max points */
	do
	{
		m_t.SetFontSize(fs);
		if(m_t.GetWidth()>pw)
			break;
		++fs;
	}while(fs<14);
	lh=m_t.GetLineHeight();				/* height of 1 line of text */
	cx=((wc->lx+wc->rx)/2)-m_txpix;
	y=((wc->by+wc->ty-lh)/2)-m_typix;
	lw=m_t.GetWidth();
	DrawLabel(&m_t,cx-(lw/2.0f),y,lw,lh,0.0f,false);
}
#endif

/* used for drawing the sides of the bridge and sides of the tunnel */
void OSMMap::Translate(unsigned int numpoints,kGUIFPoint2 *in,kGUIFPoint2 *out,float angle,float length)
{
	unsigned int i;
	float oldheading,newheading,avgheading;
	kGUIFPoint2 *p1;
	kGUIFPoint2 *p2;

	p1=in;
	p2=in+1;
	oldheading=atan2f(p2->y-p1->y,p2->x-p1->x);
	for(i=0;i<numpoints;++i)
	{
		if(!i)
			avgheading=newheading=oldheading;
		else if(i==(numpoints-1))
			avgheading=newheading;
		else
		{
			newheading=atan2f(p2->y-p1->y,p2->x-p1->x);
			avgheading=atan2(sin(oldheading)+sin(newheading),cos(oldheading)+cos(newheading));
		}

		out->x=p1->x-(cos(avgheading+angle)*length);
		out->y=p1->y-(sin(avgheading+angle)*length);
		++p1;
		++p2;
		++out;
		oldheading=newheading;
	}
}

/* draw tile at this position */
int OSMMap::DrawTile(int tx,int ty)
{
	unsigned int i;
	unsigned int j;
	unsigned int si;
	int nx,ny,lnx=0,lny=0,pixlen;
	kGUICorners tc;
	kGUICorners subcorners;
	kGUICorners wc;
	OSMSECNODE_DEF *n;
	OSMSECWAY_DEF *w;
	OSMSection *s;
	const OSMRENDER_INFO *r;
	float thickness,edge;
	int maxplwidth;
	bool save;

	/* temp heap used to save projected coords for multi-pass rendering */
	m_numdrawways=0;

	m_drawheap.Reset();
	m_lc.Clear();

	m_txpix=tx<<8;
	m_typix=ty<<8;

	/* add a bit of overlap to handle fat polylines */
	maxplwidth=10;

	tc.lx=m_txpix-maxplwidth;
	tc.rx=m_txpix+256+maxplwidth;
	tc.ty=m_typix-maxplwidth;
	tc.by=m_typix+256+maxplwidth;

	kGUI::DrawRect(0,0,256,256,DrawColor(242,239,233));

	for(si=0;si<m_numsections;++si)
	{
		s=m_sections.GetEntryPtr(si);
		s->m_current=false;

		/* check to see if we overlap this section */
		ToMap(&s->m_nw,&subcorners.lx,&subcorners.ty);
		ToMap(&s->m_se,&subcorners.rx,&subcorners.by);
			
		if(kGUI::Overlap(&tc,&subcorners))
		{
			/* load img file, draw it */
			if(!s->m_loaded)
			{
				DataHandle dh;

				dh.SetFilename(&m_filename);
				if(dh.Open())
				{
					s->Load(&dh);
					dh.Close();
				}
			}

#if 0
			//debugging!
			{
				static kGUIColor debcolors[]={
					DrawColor(255,0,0),
					DrawColor(0,255,0),
					DrawColor(0,0,255),
					DrawColor(255,255,0),
					DrawColor(255,0,255),
					DrawColor(0,255,255)};

				int debcindex=si%6;
				kGUI::DrawRect(subcorners.lx-m_txpix,subcorners.ty-m_typix,subcorners.rx-m_txpix,subcorners.by-m_typix,debcolors[debcindex]);
			}
#endif

			s->m_current=true;
			/* draw all ways that overlap this tile */
			w=s->m_way;
			for(i=0;i<s->m_numways;++i)
			{
				/* project the corners so we can clip check */
				ToMap(&(w->m_min),&wc.lx,&wc.by);
				ToMap(&(w->m_max),&wc.rx,&wc.ty);
				
				if(kGUI::Overlap(&tc,&wc))
				{
					/* draw this way since it overlaps the tile */

					/* todo: remove duplicate points that are one after another */
					pixlen=0;
					for(j=0;j<w->m_numcoords;++j)
					{
						ToMap(&w->m_coords[j],&nx,&ny);
						m_ppoints[j].x=(float)(nx-m_txpix);
						m_ppoints[j].y=(float)(ny-m_typix);

						if(j)
							pixlen+=(int)hypot(nx-lnx,ny-lny);
						lnx=nx;
						lny=ny;
					}
					r=w->m_render;
					save=false;
					switch(r->rendertype)
					{
					case OSMRENDERTYPE_POLY:
						if(w->m_name)
							save=true;	/* save for name rendering pass */
						if(w->m_closed)
							kGUI::DrawPoly(w->m_numcoords,m_ppoints,r->colour);
						else
							kGUI::DrawPolyLine(w->m_numcoords,m_ppoints,r->colour);
#if DRAWDEBUG
						DebugDrawType(w,&wc);
#endif
					break;
					case OSMRENDERTYPE_OUTPOLY:
						save=true;	/* save for color and name rendering pass */
						if(w->m_closed)
						{
							/* since we are not sure if it is clockwise or counter clockwise */
							/* we will expand both left and right */
							/* todo: save cw/ccw flag when exporting?? */

							thickness=osmthickinfo[OSMLINETYPE_STREET][GetZoom()];
							edge=valmin(valmax(0.75f,thickness*0.115f),1.5f);

							Translate(w->m_numcoords,m_ppoints,m_ppoints2,PI/2,edge);
							kGUI::DrawPoly(w->m_numcoords,m_ppoints2,r->colour2,0.66f);
							Translate(w->m_numcoords,m_ppoints,m_ppoints2,PI+(PI/2),edge);
							kGUI::DrawPoly(w->m_numcoords,m_ppoints2,r->colour2,0.66f);
						}
						else
							kGUI::DrawPolyLine(w->m_numcoords,m_ppoints,r->colour);
#if DRAWDEBUG
						DebugDrawType(w,&wc);
#endif
					break;
					case OSMRENDERTYPE_TRACK:
						if(GetZoom()>13)
							MSGPXMap::DrawTrainTracks(w->m_numcoords,m_ppoints,r->colour);
						else
							kGUI::DrawPolyLine(w->m_numcoords,m_ppoints,r->colour);
#if DRAWDEBUG
						DebugDrawType(w,&wc);
#endif
					break;
					case OSMRENDERTYPE_DOTTED:
						if(GetZoom()>13)
							MSGPXMap::DrawDottedLine(w->m_numcoords,m_ppoints,5.0f,1.5f,r->colour);
						else
							kGUI::DrawPolyLine(w->m_numcoords,m_ppoints,r->colour);
#if DRAWDEBUG
						DebugDrawType(w,&wc);
#endif
					break;
					case OSMRENDERTYPE_DASHED:
						if(GetZoom()>13)
							MSGPXMap::DrawDashedLine(w->m_numcoords,m_ppoints,3.0f,r->colour);
						else
							kGUI::DrawPolyLine(w->m_numcoords,m_ppoints,r->colour);
#if DRAWDEBUG
						DebugDrawType(w,&wc);
#endif
					break;
					case OSMRENDERTYPE_POWER:
						save=true;
					break;
					case OSMRENDERTYPE_BRIDGETRACK:
						if(GetZoom()>13)
							save=true;
						else
							kGUI::DrawPolyLine(w->m_numcoords,m_ppoints,r->colour);
					break;
					case OSMRENDERTYPE_POLYLINE:
					case OSMRENDERTYPE_BRIDGE:
					case OSMRENDERTYPE_TUNNEL:
						save=true;

						thickness=osmthickinfo[r->rendersubtype][GetZoom()];
						if(thickness<=1.0f)
							kGUI::DrawPolyLine(w->m_numcoords,m_ppoints,r->colour);
						else if(r->rendertype!=OSMRENDERTYPE_BRIDGE)
						{
							thickness=thickness*0.5f;
							edge=valmin(valmax(0.75f,thickness*0.115f),1.5f);
							kGUI::DrawFatPolyLine(w->m_closed==true?0:3,w->m_numcoords,m_ppoints,r->colour2,(float)(thickness+edge),0.66f);
						}
					break;
					case OSMRENDERTYPE_ICON:
					break;
					}
					if(save)
					{
						/* save projected points for pass 2... rendering */
						m_drawways.SetEntry(m_numdrawways++,w);

						/* copy projected points to a temporary heap so we don't need to */
						/* project them again, this is a speed optimization! */
						w->m_pixlen=pixlen;
						w->m_proj=(kGUIFPoint2 *)m_drawheap.Alloc(w->m_numcoords*sizeof(kGUIFPoint2));
						memcpy(w->m_proj,m_ppoints,w->m_numcoords*sizeof(kGUIFPoint2));
					}
				}
				++w;
			}
		}
	}

	/* pass 2 */
	for(j=0;j<m_numdrawways;++j)
	{
		w=m_drawways.GetEntry(j);

		r=w->m_render;
		switch(r->rendertype)
		{
		case OSMRENDERTYPE_POLY:
		break;
		case OSMRENDERTYPE_OUTPOLY:
			kGUI::DrawPoly(w->m_numcoords,w->m_proj,r->colour);
		break;
		case OSMRENDERTYPE_POLYLINE:
			thickness=osmthickinfo[r->rendersubtype][GetZoom()];
			if(thickness>1.0f)
				kGUI::DrawFatPolyLine(w->m_closed==true?0:3,w->m_numcoords,w->m_proj,r->colour,(float)thickness*0.5f);
		break;
		case OSMRENDERTYPE_TUNNEL:
		case OSMRENDERTYPE_BRIDGE:
		case OSMRENDERTYPE_BRIDGETRACK:
			thickness=osmthickinfo[r->rendersubtype][GetZoom()];
			if(thickness>1.0f)
			{
				float sideoff;

				thickness=thickness*0.5f;
				kGUI::DrawFatPolyLine(0,w->m_numcoords,w->m_proj,r->colour,thickness);

				edge=valmin(valmax(1.0f,thickness*0.165f),2.25f);
				sideoff=thickness+(edge/2.0f);

				/* translate points for side 1 of the bridge */
				Translate(w->m_numcoords,w->m_proj,m_ppoints,PI/2,sideoff);
				if(r->rendertype==OSMRENDERTYPE_TUNNEL)
					MSGPXMap::DrawDashedLine(w->m_numcoords,m_ppoints,3.0f,r->colour2);
				else
					kGUI::DrawFatPolyLine(0,w->m_numcoords,m_ppoints,r->colour2,edge);

				/* translate points for side 2 of the bridge */
				Translate(w->m_numcoords,w->m_proj,m_ppoints,PI+(PI/2),sideoff);
				if(r->rendertype==OSMRENDERTYPE_TUNNEL)
					MSGPXMap::DrawDashedLine(w->m_numcoords,m_ppoints,3.0f,r->colour2);
				else
					kGUI::DrawFatPolyLine(0,w->m_numcoords,m_ppoints,r->colour2,edge);
			}
			if(r->rendertype==OSMRENDERTYPE_BRIDGETRACK)
				MSGPXMap::DrawTrainTracks(w->m_numcoords,w->m_proj,DrawColor(64,64,64));
		break;
		}
	}

	/* pass 3 is text */
	for(si=0;si<m_numsections;++si)
	{
		s=m_sections.GetEntryPtr(si);
		if(s->m_current)
		{
			n=s->m_node;
			for(i=0;i<s->m_numnodes;++i)
			{
				int lw,lh;

				/* convert lat/lon to pixel */
				ToMap(&(n->c),&nx,&ny);
				if(nx>=tc.lx && nx<=tc.rx && ny>=tc.ty && ny<=tc.by)
				{
					thickness=osmthickinfo[OSMLINETYPE_TOWER][GetZoom()];
					if(thickness>=0.0f)
					{
						nx-=m_txpix;
						ny-=m_typix;
						r=n->m_render;
						kGUI::DrawRect((int)(nx-thickness),(int)(ny-thickness),(int)(nx+thickness),(int)(ny+thickness),r->colour);
						if(n->m_name)
						{
							m_t.SetFontSize((int)(thickness*4.0f));
							m_t.SetString(n->m_name);
							lh=m_t.GetLineHeight();				/* height of 1 line of text */
							ny-=(int)(lh+thickness);

							lw=m_t.GetWidth();
							DrawLabel(&m_t,nx-(lw/2.0f),ny,lw,lh,0.0f,false);
						}
					}
				}
				++n;
			}
		}
	}

	/* pass 3 */
	for(j=0;j<m_numdrawways;++j)
	{
		w=m_drawways.GetEntry(j);

		r=w->m_render;
		switch(r->rendertype)
		{
		case OSMRENDERTYPE_POLY:
		case OSMRENDERTYPE_OUTPOLY:
		{
			unsigned int pw;
			int fs,cx,y,lh,lw;

			ToMap(&(w->m_min),&wc.lx,&wc.by);
			ToMap(&(w->m_max),&wc.rx,&wc.ty);

			pw=wc.rx-wc.lx;

			m_t.SetString(w->m_name);

			fs=7;
			/* expand font size until it doesn't fit anymore or max points */
			do
			{
				m_t.SetFontSize(fs);
				if(m_t.GetWidth()>pw)
					break;
				++fs;
			}while(fs<14);
			if(fs>7 || GetZoom()==MAXOSMZOOM)
			{
				lh=m_t.GetLineHeight();				/* height of 1 line of text */
				cx=((wc.lx+wc.rx)/2)-m_txpix;
				y=((wc.by+wc.ty-lh)/2)-m_typix;

				lw=m_t.GetWidth();
				DrawLabel(&m_t,cx-(lw/2.0f),y,lw,lh,0.0f,false);
			}
#if DRAWDEBUG
			DebugDrawType(w,&wc);
#endif
		}
		break;
		case OSMRENDERTYPE_POLYLINE:
#if 0
			if(w->m_closed)
				goto treataspoly;
#endif
		case OSMRENDERTYPE_BRIDGETRACK:
		case OSMRENDERTYPE_BRIDGE:
			m_t.SetString(w->m_name);
			if(m_t.GetLen())
			{
				int fs;

				thickness=osmthickinfo[r->rendersubtype][GetZoom()];
				if(thickness==1.0f)
					thickness=4.0f;
				fs=(int)(thickness*1.1f);
				if(fs>4 || GetZoom()==MAXOSMZOOM)
				{
					m_pixlen=w->m_pixlen;
					m_t.SetFontSize(fs);
					DrawLineLabel(&m_t,w->m_numcoords,w->m_proj,-thickness*0.45f,true);
				}
			}
#if DRAWDEBUG
			ToMap(&(w->m_min),&wc.lx,&wc.by);
			ToMap(&(w->m_max),&wc.rx,&wc.ty);
			DebugDrawType(w,&wc);
#endif
		break;
		case OSMRENDERTYPE_POWER:
			kGUI::DrawPolyLine(w->m_numcoords,w->m_proj,r->colour);
			thickness=osmthickinfo[OSMLINETYPE_TOWER][GetZoom()];
			if(thickness>0.0f)
			{
				float towx,towy;

				/* draw tower square for each junction */
				for(unsigned int k=0;k<w->m_numcoords;++k)
				{
					towx=w->m_proj[k].x;
					towy=w->m_proj[k].y;
					/* top */
					kGUI::DrawLine(towx-thickness,towy-thickness,towx+thickness,towy-thickness,r->colour);
					/* bottom */
					kGUI::DrawLine(towx-thickness,towy+thickness,towx+thickness,towy+thickness,r->colour);
					/* left */
					kGUI::DrawLine(towx-thickness,towy-thickness,towx-thickness,towy+thickness,r->colour);
					/* right */
					kGUI::DrawLine(towx+thickness,towy-thickness,towx+thickness,towy+thickness,r->colour);
					/* diag1 */
					kGUI::DrawLine(towx-thickness,towy-thickness,towx+thickness,towy+thickness,r->colour);
					/* diag2 */
					kGUI::DrawLine(towx-thickness,towy+thickness,towx+thickness,towy-thickness,r->colour);
				}
			}
		break;
		}
	}

	return(TILE_OK);
}

/* draw label but first check too make sure it doesn't */
/* overlap and previously drawn labels */

void OSMMap::DrawLabel(kGUIText *t,double lx,double ly,double lw,double lh,double heading,bool clipedge)
{
	int hx,hy;
	bool dodraw;
	kGUIPoint2 corners[4];
	kGUICorners bounds;
//	kGUIImage *icon;
//	double icx=0.0f,icy=0.0f;

	/* calculate 4 corners of the text */
#if 0	
	if(m_labelicon>=0)
	{
		kGUICorners tbounds;

		icon=m_icons.GetEntry(m_labelicon);
		/* center icon over position */
		lx-=icon->GetImageWidth()>>1;
		ly-=icon->GetImageHeight()>>1;
		icx=iconcenterx[m_labelicon];
		icy=iconcentery[m_labelicon];

		/* if icon is cliped against edge of tile then don't draw */
		bounds.lx=(int)lx;
		bounds.rx=(int)lx+icon->GetImageWidth();
		bounds.ty=(int)ly;
		bounds.by=(int)ly+icon->GetImageHeight();

		tbounds.lx=0;
		tbounds.ty=0;
		tbounds.rx=256;
		tbounds.by=256;

		if(clipedge)
		{
			if(kGUI::Inside(&bounds,&tbounds)==false)
				return;
		}
		else
		{
			if(kGUI::Overlap(&bounds,&tbounds)==false)
				return;
		}
	}
	else
		icon=0;
#endif

	if(heading==0.0f)
	{
		corners[0].x=(int)lx;
		corners[0].y=(int)ly;
		corners[1].x=(int)(lx+lw);
		corners[1].y=(int)ly;
		corners[2].x=(int)(lx+lw);
		corners[2].y=(int)(ly+lh);
		corners[3].x=(int)lx;
		corners[3].y=(int)(ly+lh);
		
		bounds.lx=(int)lx;
		bounds.rx=(int)(lx+lw);
		bounds.ty=(int)ly;
		bounds.by=(int)(ly+lh);
	}
	else
	{
		corners[0].x=(int)lx;
		corners[0].y=(int)ly;
		corners[1].x=(int)(lx+(int)(cos(heading)*lw));
		corners[1].y=(int)(ly-(int)(sin(heading)*lw));
		hx=(int)(cos(heading-(PI*2*0.25f))*lh);
		hy=(int)(sin(heading-(PI*2*0.25f))*lh);
		corners[3].x=corners[0].x+hx;
		corners[3].y=corners[0].y-hy;
		corners[2].x=corners[1].x+hx;
		corners[2].y=corners[1].y-hy;

		bounds.lx=valmin(valmin(corners[0].x,corners[1].x),
						valmin(corners[2].x,corners[3].x));
		bounds.ty=valmin(valmin(corners[0].y,corners[1].y),
						valmin(corners[2].y,corners[3].y));
		bounds.rx=valmax(valmax(corners[0].x,corners[1].x),
						valmax(corners[2].x,corners[3].x));
		bounds.by=valmax(valmax(corners[0].y,corners[1].y),
						valmax(corners[2].y,corners[3].y));
	}

	dodraw=m_lc.Check(&bounds,corners,clipedge);

	if(dodraw==true)
	{
		if(heading==0.0f)
		{
#if 0
//0x2A Interstate highway shield
//0x2B U.S. highway shield
//0x2C State highway (circle)
//0x2D Canadian national highway, blue and red box
//0x2E Canadian national highway, black and white box
//0x2F Highway with small, white box
			if(icon)
			{
				ICON_POS *ipp;
				ICON_POS ip;

				/* if icon too close to another one for same highway then ignore, else add to list */
				for(i=0;(i<m_numiconsdrawn) && icon;++i)
				{
					ipp=m_iconpos.GetEntryPtr(i);
					if(ipp->icon==m_labelicon)
					{
						if(hypot(ipp->x-lx,ipp->y-ly)<128.0f)
							return;
					}
				}

				ip.x=(int)lx;
				ip.y=(int)ly;
				ip.icon=m_labelicon;
				m_iconpos.SetEntry(m_numiconsdrawn++,ip);

				if((lw+4)>icon->GetImageWidth())
				{
					icon->SetScale((double)(lw+4)/(double)icon->GetImageWidth(),1.0f);
					icon->Draw(0,(int)lx,(int)ly);
					ly+=icy-(lh/2);
					lx+=2;
				}
				else
				{
					icon->SetScale(1.0f,1.0f);
					icon->Draw(0,(int)lx,(int)ly);
					lx+=icx-(lw/2);
					ly+=icy-(lh/2);
				}

			}
#endif
			t->DrawRot((float)lx,(float)ly,0.0f,DrawColor(0,0,0),1.0f);
		}
		else
			t->DrawRot((float)lx,(float)ly,(float)heading,DrawColor(0,0,0),1.0f);
	}
}

void OSMMap::DrawLineLabel(kGUIText *t,int nvert,kGUIFPoint2 *point,double over,bool root)
{
	int s;
	int lsec;								/* longest section */
	double x1,y1,x2,y2;
	int nc=t->GetLen();						/* get length of label in characters */
	double lw=(double)t->GetWidth();		/* get width of label in pixels */
	double lh=(double)t->GetLineHeight();	/* get height of label in pixels */
	kGUIFPoint2 *p;
	double ldist;							/* longest distance */
	double dist;
	double cx,cy;
	double heading;

	if(!nc)
		return;	/* no string! */

	if((root==true) && (m_pixlen>400))
	{
		int pixcount=0;
		int ss=0;
		int numsplits=m_pixlen/200;
		int persplit;

		/* split line into sections */

		persplit=m_pixlen/numsplits;
		for(s=1;s<nvert;++s)
		{
			pixcount+=(int)hypot(point[s-1].y-point[s].y,point[s-1].x-point[s].x);
			if(pixcount>persplit)
			{
				pixcount-=persplit;
				DrawLineLabel(t,s-ss,point+ss,over,false);
				ss=s;
			}
		}

		return;
	}

	/* calculate longest section */

	lsec=0;
	ldist=hypot(point[0].y-point[1].y,point[0].x-point[1].x);
	p=point+1;

	for(s=1;s<nvert-1;++s)
	{
		dist=hypot((p->y)-((p+1)->y),(p->x)-((p+1)->x));
		if(dist>ldist)
		{
			ldist=dist;
			lsec=s;
		}
		++p;
	}

	if(lw>(ldist*2.0f))
		return;	/* section is not long enough */

	x1=point[lsec].x;
	y1=point[lsec].y;
	x2=point[lsec+1].x;
	y2=point[lsec+1].y;
	heading=atan2((point[lsec+1].y-point[lsec].y),(point[lsec].x-point[lsec+1].x));
	if(heading<0.0f)
		heading+=2*PI;

	if(heading>=(PI/2.0f) && heading<=((3.0f*PI)/2.0f))
		heading+=PI;

	/* start at center of longest segment */
	cx=(x1+x2)/2;
	cy=(y1+y2)/2;

	/* if this is a highway icon then don't rotate and center it over the road */
//	if(m_labelicon>=0)
//		heading=0;
//	else
	{
		/* back 1/2 length of string */
		cx-=(cos(heading)*(lw*0.5f));
		cy+=(sin(heading)*(lw*0.5f));

		/* move a few pixels below the road */
		cx+=(cos(heading-(PI*2*0.25f))*over);
		cy-=(sin(heading-(PI*2*0.25f))*over);
	}
	DrawLabel(t,cx,cy,lw,lh,heading,true);
}

OSMMap::~OSMMap()
{

}

void OSMMap::ToMap(GPXCoord *c,int *sx,int *sy)
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
void OSMMap::FromMap(int sx,int sy,GPXCoord *c)
{
	double e;
	int z=GetZoom();

	c->SetLon(((double)sx - (m_bitmapOrigo[z]*256.0f)) / (m_pixelsPerLonDegree[z]*256.0f));
	e = ((double)sy - (m_bitmapOrigo[z]*256.0f)) / (m_negpixelsPerLonRadian[z]*256.0f);
	c->SetLat((2.0f * atan(exp(e)) - 3.1415926535f / 2.0f) / (3.1415926535f/180.0f));
}

/****************************************************************************/

OSMConvert::OSMConvert(const char *filename)
{
	Init();
	m_filename.SetString(filename);

	m_busyrect.SetIsBar(false);
	m_busyrect.SetPos(2,2);
	m_busyrect.SetSize(500,16);
	m_window.AddObject(&m_busyrect);

	m_status.SetPos(2,26+2+8);
	m_status.SetSize(500,300);
	m_status.SetLocked(true);	/* not editable */
	m_window.AddObject(&m_status);

	m_stop.SetPos(500-75,m_status.GetZoneBY()+8);
	m_stop.SetString("Stop");
	m_stop.SetSize(75,20);
	m_window.AddObject(&m_stop);

	m_window.GetTitle()->SetString("Converting...");
	m_window.SetSize(10,10);
	m_window.ExpandToFit();
	m_window.Center();
	m_window.SetTop(true);
	kGUI::AddWindow(&m_window);

	kGUI::AddEvent(this,CALLBACKNAME(Update));
	m_window.SetEventHandler(this,CALLBACKNAME(WindowEvent));
	m_stop.SetEventHandler(this,CALLBACKNAME(StopEvent));

	m_comm.Init(32);
	/* start the download thread */
	m_thread.Start(this,CALLBACKNAME(ConvertThread));
}

void OSMConvert::Update(void)
{
	kGUIString *s=0;

	if(m_thread.GetActive()==true)
		m_busyrect.Animate();

	/* get messages from Async thread and draw em in the status window */
	while(m_comm.GetIsEmpty()!=true)
	{
		m_comm.Read(&s);
		m_status.Append(s);
		delete s;
		m_status.MoveCursorRow(1);	/* move cursor down as lines are added */
	}

	/* if the thread is finished then change 'stop'  to 'done' */
	if(m_thread.GetActive()==false)
	{
		if(m_abort==false)
			m_stop.SetString("done");
		m_abort=true;
	}
}

OSMConvertSection::OSMConvertSection(OSMConvert *parent)
{
	m_parent=parent;
	m_numnodes=0;
	m_numways=0;
	m_nodeptrs.Init(65536,-1);
	m_wayptrs.Init(65536,-1);
}

enum
{
SPLIT_LAT,
SPLIT_LON};

enum
{
SIDE_ON=0,
SIDE_LEFT=1,
SIDE_RIGHT=2};

/* recursively split sections until each section is the desired size */
void OSMConvertSection::Split(void)
{
	unsigned int i;
	unsigned int j;
	OSMConvertSection *s1;
	OSMConvertSection *s2;
	int split;
	double splitplane;
	OSMCONVNODE_DEF *np;
	OSMCONVWAY_DEF *wp;
	OSMCONVWAY_DEF w1;
	OSMCONVWAY_DEF w2;
	int side,newside=0;
	OSMCONVNODE_DEF *onp;
	double dlon,dlat,slope,newlat=0.0f,newlon=0.0f;
	OSMCONVNODE_DEF newnode;
	bool cut;
	unsigned int nn;
//	kGUIString *s;

	/* calc bounds for section */
	for(i=0;i<m_numnodes;++i)
	{
		np=m_nodeptrs.GetEntry(i);
		if(!i)
		{
			m_minlat=m_maxlat=np->m_lat;
			m_minlon=m_maxlon=np->m_lon;
		}
		else
		{
			if(np->m_lat<m_minlat)
				m_minlat=np->m_lat;
			if(np->m_lat>m_maxlat)
				m_maxlat=np->m_lat;

			if(np->m_lon<m_minlon)
				m_minlon=np->m_lon;
			if(np->m_lon>m_maxlon)
				m_maxlon=np->m_lon;
		}
	}
//	kGUI::Trace("Split(nodes=%d,minlat=%f,maxlat=%f,minlon=%f,maxlon=%f\n",m_numnodes,m_minlat,m_maxlat,m_minlon,m_maxlon);

//	if(m_numnodes<65536)
	if((m_numways<512) || ((m_maxlat-m_minlat)<0.25f && (m_maxlon-m_minlon)<0.25f))
	{
		m_parent->AddSection(this);
		/* ok, add me to my parent's list of sections */
		//kGUI::Trace("Section is Small Enough, Adding!\n");
		return;
	}
	/* must split this into 2 */
	s1=new OSMConvertSection(m_parent);
	s2=new OSMConvertSection(m_parent);

	if((m_maxlat-m_minlat)>(m_maxlon-m_minlon))
	{
		split=SPLIT_LAT;
		splitplane=(m_minlat+m_maxlat)/2.0f;
	}
	else
	{
		split=SPLIT_LON;
		splitplane=(m_minlon+m_maxlon)/2.0f;
	}

	/* assign the nodes to each side */
	for(i=0;i<m_numnodes;++i)
	{
		np=m_nodeptrs.GetEntry(i);
		switch(split)
		{
		case SPLIT_LAT:
			if(np->m_lat<=splitplane)
				s1->m_nodeptrs.SetEntry(s1->m_numnodes++,np);
			if(np->m_lat>=splitplane)
				s2->m_nodeptrs.SetEntry(s2->m_numnodes++,np);
		break;
		case SPLIT_LON:
			if(np->m_lon<=splitplane)
				s1->m_nodeptrs.SetEntry(s1->m_numnodes++,np);
			if(np->m_lon>=splitplane)
				s2->m_nodeptrs.SetEntry(s2->m_numnodes++,np);
		break;
		}
	}
	//kGUI::Trace("NumNodes = s1=%d,s2=%d\n",s1->m_numnodes,s2->m_numnodes);

	/* assign the ways to each side */
	for(i=0;i<m_numways;++i)
	{
		wp=m_wayptrs.GetEntry(i);

		w1.m_numnodes=0;
		w2.m_numnodes=0;
		cut=false;
		side=SIDE_ON;
		onp=0;

		if(wp->m_closed)
			nn=wp->m_numnodes+1;
		else
			nn=wp->m_numnodes;

		//kGUI::Trace("Way #%d (%08x),nn=%d,nn=%d\n",i,wp,wp->m_numnodes,nn);

		for(j=0;j<nn;++j)
		{
			if(j==wp->m_numnodes)
				np=wp->m_nodes[0];
			else
				np=wp->m_nodes[j];
			switch(split)
			{
			case SPLIT_LAT:
				if(np->m_lat<splitplane)
					newside=SIDE_LEFT;
				else if(np->m_lat>splitplane)
					newside=SIDE_RIGHT;
				else
					newside=SIDE_ON;
			break;
			case SPLIT_LON:
				if(np->m_lon<splitplane)
					newside=SIDE_LEFT;
				else if(np->m_lon>splitplane)
					newside=SIDE_RIGHT;
				else
					newside=SIDE_ON;
			break;
			}
			//kGUI::Trace("  v[%d] %f,%f old/newside=%d:%d\n",j,np->m_lat,np->m_lon,side,newside);
			/* did we cross the split plane? */
			if(newside==SIDE_ON)
			{
				/* add to both sides */
				if(j<wp->m_numnodes)
				{
					m_parent->m_tempnodeptrs.SetEntry(w1.m_numnodes++,np);
					m_parent->m_tempnodeptrs2.SetEntry(w2.m_numnodes++,np);
				}
			}
			else
			{
				if((side|newside)==(SIDE_LEFT|SIDE_RIGHT))
				{
					dlon=onp->m_lon-np->m_lon;
					dlat=onp->m_lat-np->m_lat;

					cut=true;
					/* crossing the split plane, calc intersection point */
					switch(split)
					{
					case SPLIT_LAT:
						slope=dlon/dlat;
						newlat=splitplane;
						newlon=np->m_lon-(((np->m_lat-splitplane)/dlat)*dlon);
					break;
					case SPLIT_LON:
						slope=dlat/dlon;
						newlon=splitplane;
						newlat=np->m_lat-(((np->m_lon-splitplane)/dlon)*dlat);
					break;
					}

					newnode.m_renderindex=0;
					newnode.m_name=0;
					newnode.m_lat=newlat;
					newnode.m_lon=newlon;

					/* add node to both sides */
					onp=(OSMCONVNODE_DEF *)m_parent->m_heap.Alloc(sizeof(newnode));
					memcpy(onp,&newnode,sizeof(newnode));
					s1->m_nodeptrs.SetEntry(s1->m_numnodes++,onp);
					s2->m_nodeptrs.SetEntry(s2->m_numnodes++,onp);

					/* add to both polys */
					m_parent->m_tempnodeptrs.SetEntry(w1.m_numnodes++,onp);
					m_parent->m_tempnodeptrs2.SetEntry(w2.m_numnodes++,onp);
				}
				if(j<wp->m_numnodes)
				{
					/* add to appropriate */
					if(newside==SIDE_LEFT)
						m_parent->m_tempnodeptrs.SetEntry(w1.m_numnodes++,np);
					else
						m_parent->m_tempnodeptrs2.SetEntry(w2.m_numnodes++,np);
				}
			}
			side=newside;
			onp=np;
		}

		//kGUI::Trace("Cut=%s n1=%d,n2=%d\n",cut==false?"false":"true",w1.m_numnodes,w2.m_numnodes);

		/* no cutting, so just add to one side or the other */
		if(cut==false)
		{
			if(w1.m_numnodes==wp->m_numnodes)
				s1->m_wayptrs.SetEntry(s1->m_numways++,wp);
			else
				s2->m_wayptrs.SetEntry(s2->m_numways++,wp);
		}
		else
		{
			if(w1.m_numnodes>0)
			{
				w1.m_renderindex=wp->m_renderindex;
				w1.m_closed=wp->m_closed;
				w1.m_name=wp->m_name;

				w1.m_nodes=(OSMCONVNODE_DEF **)m_parent->m_heap.Alloc(w1.m_numnodes*sizeof(OSMCONVNODE_DEF *));
				memcpy(w1.m_nodes,m_parent->m_tempnodeptrs.GetArrayPtr(),w1.m_numnodes*sizeof(OSMCONVNODE_DEF *));

				/* add the way struct to the heap and then add it */
				wp=(OSMCONVWAY_DEF *)m_parent->m_heap.Alloc(sizeof(OSMCONVWAY_DEF));
				memcpy(wp,&w1,sizeof(OSMCONVWAY_DEF));
				s1->m_wayptrs.SetEntry(s1->m_numways++,wp);
			}
			if(w2.m_numnodes>0)
			{
				w2.m_renderindex=wp->m_renderindex;
				w2.m_closed=wp->m_closed;
				w2.m_name=wp->m_name;

				w2.m_nodes=(OSMCONVNODE_DEF **)m_parent->m_heap.Alloc(w2.m_numnodes*sizeof(OSMCONVNODE_DEF *));
				memcpy(w2.m_nodes,m_parent->m_tempnodeptrs2.GetArrayPtr(),w2.m_numnodes*sizeof(OSMCONVNODE_DEF *));

				/* add the way struct to the heap and then add it */
				wp=(OSMCONVWAY_DEF *)m_parent->m_heap.Alloc(sizeof(OSMCONVWAY_DEF));
				memcpy(wp,&w2,sizeof(OSMCONVWAY_DEF));
				s2->m_wayptrs.SetEntry(s2->m_numways++,wp);
			}
		}
	}
	//kGUI::Trace("NumWays = s1=%d,s2=%d\n",s1->m_numways,s2->m_numways);

	//kGUI::Trace("Delete Parent\n");
	delete this;
	//kGUI::Trace("Calling Split s1\n");
	s1->Split();
	//kGUI::Trace("Calling Split s2\n");
	s2->Split();
	//kGUI::Trace("Done Split\n");
}

/* write header info for section, calc size of data section */
unsigned int OSMConvertSection::WriteHeader(FILE *f,unsigned int offset)
{
	unsigned int i;
	unsigned int j;
	unsigned int r;
	OSMCONVWAY_DEF *wp;
	OSMCONVNODE_DEF *np;
    z_stream c_stream;		/* compression stream */
	DataHandle h;
	unsigned short s2;

	/* count nodes to export  */
	for(i=0;i<m_numnodes;++i)
	{
		np=m_nodeptrs.GetEntry(i);
		/* export if render is not undefined */
		np->m_export=(np->m_renderindex!=UNDEFINEDRENDERTYPE);
	}

	/* flag all nodes that are attached to ways as not to be exported */
	for(i=0;i<m_numways;++i)
	{
		wp=m_wayptrs.GetEntry(i);
		for(j=0;j<wp->m_numnodes;++j)
		{
			np=wp->m_nodes[j];
			np->m_export=false;
		}
	}

	/* count nodes to export  */
	m_numexnodes=0;
	for(i=0;i<m_numnodes;++i)
	{
		np=m_nodeptrs.GetEntry(i);
		/* export if render is not undefined */
		if(np->m_export)
			++m_numexnodes;
	}

	fwrite(&m_numexnodes,4,1,f);
	fwrite(&m_numways,4,1,f);

	fwrite(&m_minlat,sizeof(m_minlat),1,f);
	fwrite(&m_maxlat,sizeof(m_maxlat),1,f);
	fwrite(&m_minlon,sizeof(m_minlon),1,f);
	fwrite(&m_maxlon,sizeof(m_maxlon),1,f);

	/* used to convert coords from doubles to unsigned 16 bit integers */
	m_latscale=65535.0f/(m_maxlat-m_minlat);
	m_lonscale=65535.0f/(m_maxlon-m_minlon);

	fwrite(&offset,4,1,f);

	/* write section data to temp memory buffer */
	h.SetMemory();
	h.OpenWrite("wb");
	/* write out the nodes */
	for(i=0;i<m_numnodes;++i)
	{
		np=m_nodeptrs.GetEntry(i);
		if(np->m_export)
		{
			s2=(unsigned short)((np->m_lat-m_minlat)*m_latscale);
			h.Write((void *)&s2,(unsigned long)(sizeof(s2)));
			s2=(unsigned short)((np->m_lon-m_minlon)*m_lonscale);
			h.Write((void *)&s2,(unsigned long)(sizeof(s2)));
			s2=np->m_renderindex;
			if(np->m_name)
				s2|=32768;
			h.Write((void *)&s2,(unsigned long)(sizeof(s2)));
			if(np->m_name)
				h.Write((void *)np->m_name,(unsigned long)(strlen(np->m_name)+1));
		}
	}

	/* write out the ways, these are written out in render order, so we can either: */
	/* 1) Sort them */
	/* 2) or do multiple passes, we will do this for now as it is easier */
	
	for(r=0;r<NUMRENDERTYPES;++r)
	{
		for(i=0;i<m_numways;++i)
		{
			wp=m_wayptrs.GetEntry(i);
			if(wp->m_renderindex==r)
			{
				s2=wp->m_renderindex;
				if(wp->m_name)
					s2|=32768;		/* set the name bit */
				if(wp->m_closed)
					s2|=16384;		/* set the closed bit */

				h.Write((void *)&s2,(unsigned long)(sizeof(s2)));
				s2=wp->m_numnodes;
				h.Write((void *)&s2,(unsigned long)(sizeof(s2)));

				for(j=0;j<wp->m_numnodes;++j)
				{
					np=wp->m_nodes[j];
					s2=(unsigned short)((np->m_lat-m_minlat)*m_latscale);
					h.Write((void *)&s2,(unsigned long)(sizeof(s2)));
					s2=(unsigned short)((np->m_lon-m_minlon)*m_lonscale);
					h.Write((void *)&s2,(unsigned long)(sizeof(s2)));
				}
				if(wp->m_name)
					h.Write((void *)wp->m_name,(unsigned long)(strlen(wp->m_name)+1));
			}
		}
	}
	h.Close();

	m_unpackedlength=(unsigned int)h.GetSize();
	m_packedbuffer.Alloc(m_unpackedlength);

	c_stream.zalloc = (alloc_func)0;
	c_stream.zfree = (free_func)0;
	c_stream.opaque = (voidpf)0;

	c_stream.next_in  = (unsigned char *)h.GetBufferPtr();
	c_stream.avail_in= m_unpackedlength;
	c_stream.next_out = m_packedbuffer.GetArrayPtr();
	c_stream.avail_out=m_unpackedlength;

	deflateInit(&c_stream, 9);
	deflate(&c_stream,Z_FINISH);
	deflateEnd(&c_stream);

	m_packedlength=c_stream.total_out;

	fwrite(&m_packedlength,4,1,f);
	fwrite(&m_unpackedlength,4,1,f);

	return(m_packedlength);
}

void OSMConvertSection::Write(FILE *f)
{
	fwrite(m_packedbuffer.GetArrayPtr(),m_packedlength,1,f);
}

OSMConvertSection::~OSMConvertSection()
{

}

void OSMConvert::ConvertThread(void)
{
	kGUIString *s;
	kGUIDate start;
	kGUIDate end;
	unsigned int i;
	kGUIString t;
	FILE *f;

	m_numsections=0;
	m_section.Init(1024,-1);

	s=new kGUIString();
	s->Sprintf("Starting '%s'\n",m_filename.GetString());
	while(m_comm.Write(&s)==false);

	start.SetToday();
	//step1 - load stuff into memory!
	//load into the root section
	m_rs=new OSMConvertSection(this);
	StreamLoad(m_filename.GetString());

	/* is there also a shapefile?? */
	t.SetString(m_filename.GetString());
	t.Replace(".osm","_coastline.shp");
	f=fopen(t.GetString(),"rb");
	if(f)
	{
		LoadShapefile(f);
		fclose(f);
	}

	s=new kGUIString();
	s->Sprintf("NumNodes=%d, NumWays=%d\n",m_rs->m_numnodes,m_rs->m_numways);
	while(m_comm.Write(&s)==false);

	end.SetToday();
	s=new kGUIString();
	s->Sprintf("Elapsed Load time is %d minutes, %d seconds\n",start.GetDiffMinutes(&end),start.GetDiffSeconds(&end)%60);
	while(m_comm.Write(&s)==false);

	start.SetToday();
	//recursively split into smaller sections as necessary!
	m_rs->Split();

	//step3 - write out multiple files for each area!
	end.SetToday();
	s=new kGUIString();
	s->Sprintf("Map Section Split Time is %d minutes, %d seconds\n",start.GetDiffMinutes(&end),start.GetDiffSeconds(&end)%60);
	while(m_comm.Write(&s)==false);

	start.SetToday();
	/* save it out as a binary file */
	t.SetString(&m_filename);
	t.Replace(".osm",".osb");

	s=new kGUIString();
	s->Sprintf("Writing File '%s'\n",t.GetString());
	while(m_comm.Write(&s)==false);

	f=fopen(t.GetString(),"wb");
	if(f)
	{
		unsigned int offset;

		fwrite("osmb",4,1,f);
		fwrite(osmver,4,1,f);			/* version */

		fwrite(&m_numsections,4,1,f);	/* number of sections */
		offset=12+(m_numsections*SECTIONHEADERSIZE);
		for(unsigned int z=0;z<m_numsections;++z)
			offset+=m_section.GetEntry(z)->WriteHeader(f,offset);
		for(unsigned int z=0;z<m_numsections;++z)
			m_section.GetEntry(z)->Write(f);
		fclose(f);
	}

	end.SetToday();
	s=new kGUIString();
	s->Sprintf("Writing Time is %d minutes, %d seconds\n",start.GetDiffMinutes(&end),start.GetDiffSeconds(&end)%60);
	while(m_comm.Write(&s)==false);

	PrintUnknownPairs();
	PrintUnknownRender();

	/* delete all sections */
	s=new kGUIString();
	s->Sprintf("Freeing Sections!\n");
	while(m_comm.Write(&s)==false);

	for(i=0;i<m_numsections;++i)
	{
	//	s=new kGUIString();
	//	s->Sprintf("Deleting section #%d\n",i);
	//	while(m_comm.Write(&s)==false);
		delete m_section.GetEntry(i);
	}
	s=new kGUIString();
	s->Sprintf("Done Freeing Sections!\n");
	while(m_comm.Write(&s)==false);

	m_abort=false;

	do
	{
		kGUI::Sleep(1);
		if(m_abort)
			break;
	}while(1);
	m_thread.Close(true);
}

/* big endian load */
static unsigned int Load32B(FILE *f)
{
	unsigned int i;
	unsigned int v;
	unsigned char c;

	v=0;
	for(i=0;i<4;++i)
	{
		fread(&c,1,1,f);
		v=(v<<8)|c;
	}
	return(v);
}

/* load shapefile */
void OSMConvert::LoadShapefile(FILE *f)
{
	unsigned int i;
	unsigned int recordnum,type;
	int recsize,filesize;
	unsigned int i32;
	double minx,miny,maxx,maxy,minz,maxz,minm,maxm;

	i32=Load32B(f);
	kGUI::Trace("shapefile header=%08x\n",i32);
	if(i32!=0x0000270a)
		return;

	/* 5 unused ints */
	for(i=0;i<5;++i)
		fread(&i32,4,1,f);

	filesize=Load32B(f);
	kGUI::Trace("shapefile filelength=%d\n",filesize);

	fread(&i32,4,1,f);
	kGUI::Trace("shapefile version=%08x\n",i32);

	fread(&i32,4,1,f);
	kGUI::Trace("shapefile type=%08x\n",i32);

	fread(&minx,sizeof(minx),1,f);
	kGUI::Trace("shapefile minx=%f\n",minx);

	fread(&miny,sizeof(miny),1,f);
	kGUI::Trace("shapefile miny=%f\n",miny);

	fread(&maxx,sizeof(maxx),1,f);
	kGUI::Trace("shapefile maxx=%f\n",maxx);

	fread(&maxy,sizeof(maxy),1,f);
	kGUI::Trace("shapefile maxy=%f\n",maxy);

	fread(&minz,sizeof(minz),1,f);
	kGUI::Trace("shapefile minz=%f\n",minz);

	fread(&maxz,sizeof(maxz),1,f);
	kGUI::Trace("shapefile maxz=%f\n",maxz);

	fread(&minm,sizeof(minm),1,f);
	kGUI::Trace("shapefile minm=%f\n",minm);

	fread(&maxm,sizeof(maxm),1,f);
	kGUI::Trace("shapefile maxm=%f\n",maxm);
	filesize-=50;

	/* records */
	
	recordnum=1;
	do
	{
		i32=Load32B(f);
		kGUI::Trace("shapefile recordnum=%08x\n",i32);
		if(i32!=recordnum)
		{
			kGUI::Trace("recordnum mismatch\n");
			for(i=0;i<20;++i)
			{
				fread(&i32,4,1,f);
				kGUI::Trace("%08x\n",i32);
			}
			return;
		}
		recsize=Load32B(f);
		kGUI::Trace("shapefile recordsize=%d, filesize=%d\n",recsize,filesize);
		filesize-=(recsize+4);

		/* get the type */
		fread(&type,4,1,f);
		kGUI::Trace("type=%08x\n",type);
		recsize-=2;

		while(recsize>0)
		{
			fread(&i32,4,1,f);
			kGUI::Trace("%08x\n",i32);
			recsize-=2;
		};
		++recordnum;
	}while(filesize>0);
}

void OSMConvert::WindowEvent(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_CLOSE:
		delete this;
	break;
	}
}

void OSMConvert::StopEvent(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_PRESSED:
		m_window.Close();
	break;
	}
}

OSMConvert::~OSMConvert()
{
	kGUIString *s=0;

	/* wait for finished */
	m_abort=true;
	while(m_thread.GetActive());

	kGUI::DelEvent(this,CALLBACKNAME(Update));

	/* delete any dangling strings */
	while(m_comm.GetIsEmpty()!=true)
	{
		m_comm.Read(&s);
		delete s;
	}

	kGUI::DelWindow(&m_window);
}

static const OSMTAG_DEF osmtaglist[]={
	{OSMTAG_NODE,		"node"},
	{OSMTAG_WAY,		"way"},
	{OSMTAG_ND,			"nd"},
	{OSMTAG_LON,		"lon"},
	{OSMTAG_LAT,		"lat"},
	{OSMTAG_TAG,		"tag"},
	{OSMTAG_K,			"k"},
	{OSMTAG_V,			"v"},
	{OSMTAG_TIMESTAMP,	"timestamp"},
	{OSMTAG_USER,		"user"},
	{OSMTAG_VISIBLE,	"visible"},
	{OSMTAG_ID,			"id"}};

static const OSMTAG_DEF osmtagspecial[]={
	{OSMTAG_STOPSIGN,	"stopsign"},
	{OSMTAG_YIELDSIGN,	"yieldsign"},
	{OSMTAG_NOLEFTTURN,	"no_left_turn"},
	{OSMTAG_NOLEFTTURN,	"noleftturn"},
	{OSMTAG_NORIGHTTURN,"no_right_turn"},
	{OSMTAG_NORIGHTTURN,"norightturn"},

	{OSMTAG_PLACE_NAME,	"loc_name"},
	{OSMTAG_PLACE_NAME,	"place_name"},
	{OSMTAG_PLACE_NAME,	"name_base"},
	{OSMTAG_PLACE_NAME,	"name_1"},
	{OSMTAG_PLACE_NAME,	"name:1"},
	{OSMTAG_PLACE_NAME,	"name:2"},
	{OSMTAG_PLACE_NAME,	"name_2"},
	{OSMTAG_PLACE_NAME,	"name_3"},
	{OSMTAG_PLACE_NAME,	"name_4"},
	{OSMTAG_PLACE_NAME,	"name:en"},
	{OSMTAG_PLACE_NAME,	"name:fr"},
	{OSMTAG_PLACE_NAME,	"name:de"},
	{OSMTAG_PLACE_NAME,	"name"},
	{OSMTAG_PLACE_NAME,	"name1"},
	{OSMTAG_PLACE_NAME,	"name2"},
	{OSMTAG_PLACE_NAME,	"description"},
	{OSMTAG_PLACE_NAME,	"operator"},
	{OSMTAG_PLACE_NAME,	"cuisine"},
	{OSMTAG_PLACE_NAME,	"reg_name"},

	{OSMTAG_IGNORE,		"created_by"},
	{OSMTAG_IGNORE,		"time"},
	{OSMTAG_IGNORE,		"owner"},
	{OSMTAG_IGNORE,		"ownership"},
	{OSMTAG_IGNORE,		"is_in"},
	{OSMTAG_IGNORE,		"is_in:continent"},
	{OSMTAG_IGNORE,		"int_name"},
	{OSMTAG_IGNORE,		"lcn_ref"},	//maybe use after all
	{OSMTAG_IGNORE,		"loc_ref"},
	{OSMTAG_IGNORE,		"ncn_ref"},//maybe use after all
	{OSMTAG_IGNORE,		"old_ref"},
	{OSMTAG_IGNORE,		"nat_ref"},
	{OSMTAG_IGNORE,		"ref"},
	{OSMTAG_IGNORE,		"admin_level"},
	{OSMTAG_IGNORE,		"name:source"},
	{OSMTAG_IGNORE,		"house_numbers"},
	{OSMTAG_IGNORE,		"note"},
	{OSMTAG_IGNORE,		"note:2"},
	{OSMTAG_IGNORE,		"notes"},
	{OSMTAG_IGNORE,		"comment"},
	{OSMTAG_IGNORE,		"maxspeed"},
	{OSMTAG_IGNORE,		"census:population"},
	{OSMTAG_IGNORE,		"population"},
	{OSMTAG_IGNORE,		"catmp-roadid"},
	{OSMTAG_IGNORE,		"enc:lnam"},
	{OSMTAG_IGNORE,		"nist:fips_code"},
	{OSMTAG_IGNORE,		"nist:state_fips"},
	{OSMTAG_IGNORE,		"nist:garmin_road_class"},
	{OSMTAG_IGNORE,		"garmin_road_class"},
	{OSMTAG_IGNORE,		"gnis:id"},
	{OSMTAG_IGNORE,		"gnis:class"},
	{OSMTAG_IGNORE,		"gnis:county"},
	{OSMTAG_IGNORE,		"gnis:county_num"},
	{OSMTAG_IGNORE,		"gnis:st_alpha"},
	{OSMTAG_IGNORE,		"gnis:st_num"},
	{OSMTAG_IGNORE,		"gns:n:xx:full_name"},
	{OSMTAG_IGNORE,		"gns:n:xx:full_name_nd"},
	{OSMTAG_IGNORE,		"gns:n:xx:sort_name"},
	{OSMTAG_IGNORE,		"gns:v:xx:full_name"},
	{OSMTAG_IGNORE,		"gns:v:xx:sort_name"},
	{OSMTAG_IGNORE,		"gns:v:xx:full_name_nd"},
	{OSMTAG_IGNORE,		"gns:n:xx:modify_date"},
	{OSMTAG_IGNORE,		"gns:v:xx:modify_date"},
	{OSMTAG_IGNORE,		"gns:n:xx:short_form"},
	{OSMTAG_IGNORE,		"gns:n:xx:nt"},
	{OSMTAG_IGNORE,		"gns:cc1"},
	{OSMTAG_IGNORE,		"gns:rc"},
	{OSMTAG_IGNORE,		"gns:w:xx:modify_date"},
	{OSMTAG_IGNORE,		"gns:fc"},
	{OSMTAG_IGNORE,		"gns:fc"},
	{OSMTAG_IGNORE,		"gns:fc"},
	{OSMTAG_IGNORE,		"gns:fc"},
	{OSMTAG_IGNORE,		"gns:dsg"},
	{OSMTAG_IGNORE,		"gns:mgrs"},
	{OSMTAG_IGNORE,		"gns:v:xx:nt"},
	{OSMTAG_IGNORE,		"gns:ufi"},
	{OSMTAG_IGNORE,		"gns:pop"},
	{OSMTAG_IGNORE,		"gns:uni"},
	{OSMTAG_IGNORE,		"gns:jog"},
	{OSMTAG_IGNORE,		"gns:adm1"},
	{OSMTAG_IGNORE,		"gns:lat"},
	{OSMTAG_IGNORE,		"gns:long"},
	{OSMTAG_IGNORE,		"gns:dms_lat"},
	{OSMTAG_IGNORE,		"gns:dms_long"},
	{OSMTAG_IGNORE,		"addr:postcode"},
	{OSMTAG_IGNORE,		"addr:postalcode"},
	{OSMTAG_IGNORE,		"addr:street"},
	{OSMTAG_IGNORE,		"addr:city"},
	{OSMTAG_IGNORE,		"addr:housenumber"},
	{OSMTAG_IGNORE,		"building_code"},
	{OSMTAG_IGNORE,		"county:left"},
	{OSMTAG_IGNORE,		"nhd:way_id"},
	{OSMTAG_IGNORE,		"county:right"},
	{OSMTAG_IGNORE,		"to_address_left"},
	{OSMTAG_IGNORE,		"to_address_left_1"},
	{OSMTAG_IGNORE,		"to_address_left_2"},
	{OSMTAG_IGNORE,		"to_address_right"},
	{OSMTAG_IGNORE,		"to_address_right_1"},
	{OSMTAG_IGNORE,		"to_address_right_2"},
	{OSMTAG_IGNORE,		"from_address_left"},
	{OSMTAG_IGNORE,		"from_address_left_1"},
	{OSMTAG_IGNORE,		"from_address_left_2"},
	{OSMTAG_IGNORE,		"from_address_right"},
	{OSMTAG_IGNORE,		"from_address_right_1"},
	{OSMTAG_IGNORE,		"from_address_right_2"},
	{OSMTAG_IGNORE,		"county:right"},
	{OSMTAG_IGNORE,		"ele"},
	{OSMTAG_IGNORE,		"wikipedia"},
	{OSMTAG_IGNORE,		"history"},
	{OSMTAG_IGNORE,		"url"},
	{OSMTAG_IGNORE,		"website"},
	{OSMTAG_IGNORE,		"web"},
	{OSMTAG_IGNORE,		"src_url"},
	{OSMTAG_IGNORE,		"source_url"},
	{OSMTAG_IGNORE,		"checked_by"},
	{OSMTAG_IGNORE,		"download_img"},
	{OSMTAG_IGNORE,		"poi"},
	{OSMTAG_IGNORE,		"wpt_description"},
	{OSMTAG_IGNORE,		"address"},
	{OSMTAG_IGNORE,		"opening_hours"},
	{OSMTAG_IGNORE,		"comment"},
	{OSMTAG_IGNORE,		"key"},
	{OSMTAG_IGNORE,		"name_type"},
	{OSMTAG_IGNORE,		"name_type_1"},
	{OSMTAG_IGNORE,		"phone"},
	{OSMTAG_IGNORE,		"reviewed"},
	{OSMTAG_IGNORE,		"attribution"},
	{OSMTAG_IGNORE,		"telephone"},
	{OSMTAG_IGNORE,		"postal_code"},
	{OSMTAG_IGNORE,		"old_name"},
	{OSMTAG_IGNORE,		"icao"},
	{OSMTAG_IGNORE,		"import_uuid"},
	{OSMTAG_IGNORE,		"iata"},
	{OSMTAG_IGNORE,		"start_date"},
	{OSMTAG_IGNORE,		"garmin_type"},
	{OSMTAG_IGNORE,		"tiger:county"},
	{OSMTAG_IGNORE,		"tiger:cfcc"},
	{OSMTAG_IGNORE,		"tiger:tlid"},
	{OSMTAG_IGNORE,		"tiger:tzid"},
	{OSMTAG_IGNORE,		"tiger:upload_uuid"},
	{OSMTAG_IGNORE,		"tiger:name_base"},
	{OSMTAG_IGNORE,		"tiger:name_base_1"},
	{OSMTAG_IGNORE,		"tiger:name_base_2"},
	{OSMTAG_IGNORE,		"tiger:name_base_3"},
	{OSMTAG_IGNORE,		"tiger:name_type"},
	{OSMTAG_IGNORE,		"tiger:name_type_1"},
	{OSMTAG_IGNORE,		"tiger:name_type_2"},
	{OSMTAG_IGNORE,		"tiger:name_type_3"},
	{OSMTAG_IGNORE,		"tiger:name_direction_prefix"},
	{OSMTAG_IGNORE,		"tiger:name_direction_prefix_1"},
	{OSMTAG_IGNORE,		"tiger:name_direction_prefix_2"},
	{OSMTAG_IGNORE,		"tiger:name_direction_suffix"},
	{OSMTAG_IGNORE,		"tiger:name_direction_suffix_1"},
	{OSMTAG_IGNORE,		"tiger:name_direction_suffix_2"},
	{OSMTAG_IGNORE,		"tiger:separated"},
	{OSMTAG_IGNORE,		"zip_left"},
	{OSMTAG_IGNORE,		"zip_left_1"},
	{OSMTAG_IGNORE,		"zip_left_2"},
	{OSMTAG_IGNORE,		"zip_left_3"},
	{OSMTAG_IGNORE,		"tiger:zip_left"},
	{OSMTAG_IGNORE,		"tiger:zip_left_1"},
	{OSMTAG_IGNORE,		"tiger:zip_left_2"},
	{OSMTAG_IGNORE,		"tiger:zip_left_3"},
	{OSMTAG_IGNORE,		"tiger:zip_left_4"},
	{OSMTAG_IGNORE,		"tiger:zip_left_5"},
	{OSMTAG_IGNORE,		"tiger:zip_left_5"},
	{OSMTAG_IGNORE,		"tiger:zip_left_6"},
	{OSMTAG_IGNORE,		"tiger:zip_left_7"},
	{OSMTAG_IGNORE,		"tiger:zip_left_8"},
	{OSMTAG_IGNORE,		"tiger:zip_left_9"},
	{OSMTAG_IGNORE,		"zip_right"},
	{OSMTAG_IGNORE,		"zip_right_1"},
	{OSMTAG_IGNORE,		"zip_right_2"},
	{OSMTAG_IGNORE,		"zip_right_3"},
	{OSMTAG_IGNORE,		"tiger:zip_right"},
	{OSMTAG_IGNORE,		"tiger:zip_right_1"},
	{OSMTAG_IGNORE,		"tiger:zip_right_2"},
	{OSMTAG_IGNORE,		"tiger:zip_right_3"},
	{OSMTAG_IGNORE,		"tiger:zip_right_4"},
	{OSMTAG_IGNORE,		"tiger:zip_right_5"},
	{OSMTAG_IGNORE,		"tiger:zip_right_6"},
	{OSMTAG_IGNORE,		"tiger:zip_right_7"},
	{OSMTAG_IGNORE,		"tiger:zip_right_8"},
	{OSMTAG_IGNORE,		"tiger:zip_right_9"},
	{OSMTAG_IGNORE,		"tiger:upload_uid"},
	{OSMTAG_IGNORE,		"tiger:source"},
	{OSMTAG_IGNORE,		"tiger:seperated"},
	{OSMTAG_IGNORE,		"tiger:reviewed"},
	{OSMTAG_IGNORE,		"side"},
	{OSMTAG_IGNORE,		"left"},
	{OSMTAG_IGNORE,		"right"},
	{OSMTAG_IGNORE,		"left:city"},
	{OSMTAG_IGNORE,		"right:city"},
	{OSMTAG_IGNORE,		"left:province"},
	{OSMTAG_IGNORE,		"right:provice"},
	{OSMTAG_IGNORE,		"left:region"},
	{OSMTAG_IGNORE,		"right:region"},
	{OSMTAG_IGNORE,		"left:state"},
	{OSMTAG_IGNORE,		"right:state"},
	{OSMTAG_IGNORE,		"left:country"},
	{OSMTAG_IGNORE,		"right:country"},
	{OSMTAG_IGNORE,		"state:left"},
	{OSMTAG_IGNORE,		"state:right"},
	{OSMTAG_IGNORE,		"country:left"},
	{OSMTAG_IGNORE,		"country:right"},
	{OSMTAG_IGNORE,		"wdb:source"},
	{OSMTAG_IGNORE,		"osmarender:renderName"},
	{OSMTAG_IGNORE,		"fixme"},
	{OSMTAG_IGNORE,		"debug"},
	{OSMTAG_IGNORE,		"clopin:route"},
	{OSMTAG_IGNORE,		"clopin:id"},
	{OSMTAG_IGNORE,		"sourceref"},
	{OSMTAG_IGNORE,		"source_ref"},
	{OSMTAG_IGNORE,		"notes3"},
	{OSMTAG_IGNORE,		"hist_ref"},
	{OSMTAG_IGNORE,		"sfcn_ref"},
	{OSMTAG_IGNORE,		"rcn_ref"},
	{OSMTAG_IGNORE,		"nat_name"},

	{OSMTAG_IGNORE,		"massgis:DCAM_ID"},
	{OSMTAG_IGNORE,		"massgis:LEV_PROT"},
	{OSMTAG_IGNORE,		"massgis:DEED_ACRES"},
	{OSMTAG_IGNORE,		"massgis:OS_DEED_PA"},
	{OSMTAG_IGNORE,		"massgis:OS_DEED_BO"},
	{OSMTAG_IGNORE,		"massgis:FY_FUNDING"},
	{OSMTAG_IGNORE,		"massgis:ASSESS_ACR"},
	{OSMTAG_IGNORE,		"massgis:EOEAINVOLV"},
	{OSMTAG_IGNORE,		"massgis:PUB_ACCESS"},
	{OSMTAG_IGNORE,		"massgis:ARTICLE97"},
	{OSMTAG_IGNORE,		"massgis:FEESYM"},
	{OSMTAG_IGNORE,		"massgis:OWNER_TYPE"},
	{OSMTAG_IGNORE,		"massgis:FEE_OWNER"},
	{OSMTAG_IGNORE,		"massgis:way_id"},
	{OSMTAG_IGNORE,		"massgis:PRIM_PURP"},
	{OSMTAG_IGNORE,		"massgis:BASE_MAP"},
	{OSMTAG_IGNORE,		"massgis:TOWN_ID"},
	{OSMTAG_IGNORE,		"massgis:CAL_DATE_R"},
	{OSMTAG_IGNORE,		"massgis:PROJ_ID1"},

	{OSMTAG_IGNORE,		"massgis:OLI_1_TYPE"},
	{OSMTAG_IGNORE,		"massgis:OLI_1_INT"},
	{OSMTAG_IGNORE,		"massgis:OLI_1_ORG"},
	{OSMTAG_IGNORE,		"massgis:OLI_1_ABBR"},
	{OSMTAG_IGNORE,		"massgis:OLI_2_TYPE"},
	{OSMTAG_IGNORE,		"massgis:OLI_2_INT"},
	{OSMTAG_IGNORE,		"massgis:OLI_2_ORG"},
	{OSMTAG_IGNORE,		"massgis:OLI_2_ABBR"},
	{OSMTAG_IGNORE,		"massgis:SOURCE_MAP"},
	{OSMTAG_IGNORE,		"massgis:INTSYM"},
	{OSMTAG_IGNORE,		"massgis:OWNER_ABRV"},
	{OSMTAG_IGNORE,		"massgis:ASSESS_BLK"},
	{OSMTAG_IGNORE,		"massgis:ATT_DATE"},
	{OSMTAG_IGNORE,		"massgis:MANAGR_TYP"},

	{OSMTAG_IGNORE,		"bikepa_ref"},
	{OSMTAG_IGNORE,		"network"},
	{OSMTAG_IGNORE,		"line"},
	{OSMTAG_IGNORE,		"secondary_name"},

	{OSMTAG_IGNORE,		"source"}};

#if 0
fuck

#8 - 143 'massgis:SOURCE_MAP^1'
#10 - 125 'massgis:OLI_1_INT^APR'
#11 - 125 'massgis:INTSYM^CR'
#12 - 125 'massgis:OLI_1_INT^CR'
#13 - 125 'massgis:INTSYM^APR'
#14 - 124 'massgis:OLI_1_ORG^Department of Agricultural Resources'
#15 - 124 'massgis:OLI_1_ABRV^DAR'
#16 - 105 'massgis:FEE_OWNER^DCR - Division of State Parks and Recreation'
#17 - 105 'width^18.3'
#18 - 105 'massgis:OWNER_ABRV^DCRS'
#19 - 101 'massgis:ASSESS_BLK^1'
#20 - 87 'massgis:OLI_1_TYPE^L'
#21 - 81 'protected^limited'
#22 - 78 'massgis:SOURCE_TYP^NP'
#23 - 73 'massgis:SITE_NAME^Appalachian Trail Corridor'
#24 - 71 'massgis:SOURCE_MAP^TNC'
#25 - 62 'massgis:ATT_DATE^1901/01/01'
#26 - 61 'width^9.8'
#27 - 60 'massgis:OWNER_ABRV^NPS'
#28 - 60 'massgis:FEE_OWNER^National Park Service'
#29 - 57 'width^10.7'
#30 - 53 'massgis:SOURCE_TYP^DARB'
#31 - 52 'width^13.7'
#32 - 48 'historical^yes'
#33 - 46 'massgis:MANAGR_TYP^M'
#34 - 46 'width^11.0'
#35 - 46 'width^7.6'
#36 - 44 'massgis:OLI_1_ORG^Berkshire Natural Resources Council, Inc.'
#37 - 44 'massgis:ASSESS_LOT^1'
#38 - 44 'massgis:OLI_1_ABRV^BNRC'
#39 - 44 'width^12.8'
#40 - 44 'massgis:OWNER_ABRV^DFG'
#41 - 44 'massgis:FEE_OWNER^Department of Fish and Game'
#42 - 40 'massgis:SOURCE_MAP^DAR'
#43 - 40 'massgis:FEE_OWNER^City of Pittsfield'
#44 - 40 'massgis:OWNER_ABRV^M236'
#45 - 39 'massgis:ATT_DATE^1996/01/09'
#46 - 39 'width^6.1'
#47 - 36 'massgis:ATT_DATE^2005/12/16'
#48 - 35 'width^30.5'
#49 - 35 'massgis:OWNER_ABRV^BNRC'
#50 - 35 'massgis:FEE_OWNER^Berkshire Natural Resources Council, Inc.'
#51 - 35 'massgis:OLI_2_INT^CR'
#52 - 34 'massgis:OLI_2_TYPE^L'
#53 - 34 'massgis:OWNER_ABRV^TNC'
#54 - 34 'massgis:FEE_OWNER^The Nature Conservancy'
#55 - 33 'abutters^residential'
#56 - 32 'region^Western'
#57 - 30 'type^Public'
#58 - 29 'nat_ref^I-90'
#59 - 29 'massgis:ASSESS_LOT^2'
#60 - 29 'massgis:SITE_NAME^MLCT/Bluebird Hill'
#61 - 29 'massgis:ASSESS_MAP^33'
#62 - 28 'massgis:ASSESS_MAP^35'
#63 - 27 'massgis:ATT_DATE^2000/03/01'
#64 - 27 'massgis:SOURCE_MAP^PARCEL'
#65 - 27 'width^16.8'
#66 - 27 'massgis:ATT_DATE^1995/04/04'
#67 - 26 'massgis:ASSESS_MAP^11'
#68 - 25 'massgis:SOURCE_TYP^DD'
#69 - 25 'massgis:FEE_OWNER^BLUEBIRD HILL DEVELOPMENT LLC'
#70 - 25 'massgis:ALT_SITE_N^MLCT/ BLUEBIRD HILL'
#71 - 25 'line^Richmond Hill line'
#72 - 25 'massgis:ATT_DATE^2008/08/27'
#73 - 24 'massgis:SOURCE_MAP^V'
#74 - 24 'massgis:ASSESS_BLK^2'
#75 - 24 'width^11.6'
#76 - 23 'massgis:ATT_DATE^1999/02/01'
#77 - 22 'secondary_name^Macdonald-Cartier Freeway'
#78 - 22 'width^14.6'
#79 - 22 'massgis:ATT_DATE^2000/02/28'
#80 - 22 'massgis:SITE_NAME^PITTSFIELD STATE FOREST'
#81 - 22 'width^15.9'
#82 - 21 'massgis:FEE_OWNER^Town of Lenox'
#83 - 21 'massgis:ASSESS_MAP^7'
#84 - 21 'line^Lakeshore West line'
#85 - 21 'service^yard'
#86 - 21 'massgis:OWNER_ABRV^M152'
#87 - 21 'width^8.5'
#88 - 20 'is_in:country^USA'
#89 - 20 'massgis:ATT_DATE^2005/12/12'
#90 - 20 'line^Barrie Line'
#92 - 20 'massgis:ATT_DATE^2008/11/07'
#93 - 20 'massgis:ATT_DATE^2000/08/28'
#94 - 19 'width^13.4'
#95 - 19 'massgis:ATT_DATE^2003/09/26'
#96 - 19 'massgis:ATT_DATE^1996/11/01'
#97 - 19 'width^24.4'
#98 - 19 'massgis:ASSESS_MAP^2'
#99 - 19 'massgis:GRANTTYPE1^S'
#100 - 18 'shop^clothes'
#101 - 18 'width^7.3'
#102 - 18 'marked_trail^orange'
#103 - 18 'railway^Viva Blue'
#104 - 18 'width^91.5'
#105 - 17 'massgis:OWNER_ABRV^M113'
#106 - 17 'massgis:OLI_1_ABRV^DFG'
#107 - 17 'massgis:SOURCE_TYP^LT'
#108 - 17 'massgis:OLI_1_ORG^Department of Fish and Game'
#109 - 17 'massgis:SOURCE_TYP^SV'
#110 - 17 'massgis:FEE_OWNER^Town of Great Barrington'
#111 - 17 'massgis:ASSESS_LOT^3'
#112 - 16 'massgis:FEE_OWNER^The Trustees of Reservations'
#113 - 16 'massgis:OWNER_ABRV^TTOR'
#114 - 16 'massgis:ATT_DATE^2008/11/03'
#115 - 16 'massgis:ASSESS_MAP^4'
#116 - 16 'massgis:SOURCE_TYP^OS'
#117 - 16 'massgis:ASSESS_LOT^4'
#118 - 16 'width^10.4'
#119 - 16 'massgis:ASSESS_LOT^5'
#120 - 15 'massgis:FEE_OWNER^WILD BILL'
#121 - 15 'massgis:SITE_NAME^HIGHLAWN FARM'
#122 - 15 'massgis:ATT_DATE^1997/02/04'
#123 - 15 'massgis:ATT_DATE^2008/11/06'
#124 - 15 'DCNR^State_Forest'
#125 - 15 'massgis:OWNER_ABRV^M283'
#126 - 15 'massgis:SITE_NAME^BEARTOWN STATE FOREST'
#127 - 15 'massgis:ASSESS_LOT^7'
#128 - 15 'motorcar^private'
#129 - 15 'massgis:FEE_OWNER^Town of Stockbridge'
#130 - 14 'massgis:FEE_OWNER^X'
#131 - 14 'width^narrow'
#132 - 14 'massgis:ATT_DATE^1995/07/10'
#133 - 14 'massgis:SOURCE_MAP^DAR/Town'
#134 - 13 'massgis:ATT_DATE^2008/11/10'
#135 - 13 'massgis:ATT_DATE^1997/04/30'
#136 - 12 'type^Private'
#137 - 12 'massgis:ATT_DATE^2008/10/30'
#138 - 12 'massgis:OLI_1_ORG^The Nature Conservancy'
#139 - 12 'college^Victoria College'
#140 - 12 'massgis:ASSESS_MAP^12'
#141 - 12 'width^13.1'
#142 - 12 'width^27.4'
#143 - 12 'massgis:ASSESS_BLK^4'
#144 - 12 'massgis:SITE_NAME^EAST MOUNTAIN STATE FOREST'
#145 - 12 'massgis:SITE_NAME^HANCOCK SHAKER VILLAGE'
#146 - 12 'massgis:SITE_NAME^MT WASHINGTON STATE FOREST'
#147 - 12 'massgis:ASSESS_MAP^3'
#148 - 12 'massgis:OLI_1_ABRV^TNC'
#149 - 12 'jurisdiction^Local'
#153 - 11 'amenity^retirement_home'
#154 - 11 'nat_ref^I-390'
#155 - 11 'massgis:GRANTPROG1^USH'
#157 - 11 'piste:type^downhill'
#158 - 11 'office^Headquarters'
#165 - 11 'massgis:FEE_OWNER^DONNELLEY STRACHAN AND VIVIAN'
#166 - 11 'massgis:OLI_2_ORG^New Marlborough Land Preservation Trust'
#170 - 11 'width^14.0'
#172 - 11 'atm^yes'
#177 - 11 'massgis:ATT_DATE^2003/03/19'
#181 - 11 'massgis:OLI_2_ABRV^NMLPT'
#185 - 11 'shop^car_repair'
#188 - 11 'massgis:SITE_NAME^DONNELLY CR'
#194 - 11 'sac_scale^hiking'
#195 - 11 'shop^video_rental'
#196 - 11 'massgis:OLI_2_ABRV^BNRC'
#199 - 11 'massgis:SITE_NAME^TACONIC TRAIL STATE FOREST'
#202 - 11 'massgis:ATT_DATE^2004/11/04'
#205 - 11 'massgis:SITE_NAME^THREE MILE POND WMA'
#212 - 11 'shop^books'
#218 - 11 'massgis:SOURCE_TYP^O'
#221 - 10 'massgis:OLI_1_ABRV^SFLT'
#222 - 10 'massgis:OLI_1_ORG^Sheffield Land Trust'
#226 - 10 'massgis:ASSESS_LOT^13'
#227 - 10 'width^30.2'
#229 - 10 'flashing_light^yes'
#232 - 10 'width^7.9'
#233 - 10 'width^10.1'
#235 - 10 'massgis:MANAGR_ABR^M236CC'
#237 - 10 'massgis:ASSESS_LOT^17'
#240 - 10 'massgis:OWNER_ABRV^M090'
#241 - 10 'shop^unknown'
#242 - 10 'massgis:FEE_OWNER^LARKIN JAMES AND MARGARET'
#243 - 10 'massgis:ASSESS_LOT^10'
#244 - 10 'massgis:COMMENTS^DCS, SEVERAL LOTS WITH TOTAL 150 ACRES EXTENDING INTO SHEFFIELD (CR31), PASSIVE RECREATION ON TRAIL'
#245 - 10 'massgis:OWNER_ABRV^HSV'
#246 - 10 'massgis:ASSESS_MAP^9'
#247 - 10 'massgis:FEE_OWNER^Town of Egremont'
#248 - 10 'massgis:ATT_DATE^2008/08/21'
#250 - 10 'traffic_calming^bump'
#251 - 10 'massgis:ASSESS_SUB^B'
#252 - 10 'maxweight^10t'
#253 - 10 'massgis:SITE_NAME^MT. PLANTAIN WCE'
#257 - 9 'massgis:ASSESS_MAP^20'
#258 - 9 'massgis:SOURCE_TYP^CS'
#259 - 9 'massgis:ASSESS_MAP^5'
#262 - 9 'massgis:ATT_DATE^1997/04/29'
#263 - 9 'amenity^emergency_phone'
#264 - 9 'massgis:FEE_OWNER^Hancock Shaker Village'
#265 - 9 'massgis:ASSESS_LOT^6'
#266 - 9 'red_light_camera^yes'
#267 - 9 'width^11.3'
#268 - 9 'massgis:SOURCE_MAP^AGI'
#269 - 9 'massgis:ASSESS_MAP^1'
#270 - 9 'massgis:SOURCE_TYP^WEB'
#271 - 9 'massgis:ASSESS_LOT^9'
#272 - 9 'massgis:OLI_2_ORG^Berkshire Natural Resources Council, Inc.'
#273 - 9 'massgis:OWNER_ABRV^M150'
#274 - 9 'massgis:ATT_DATE^2008/08/08'
#275 - 9 'massgis:SOURCE_MAP^DCS'
#277 - 9 'massgis:FEE_OWNER^Town of Lee'
#278 - 9 'massgis:SOURCE_MAP^BNRC'
#279 - 9 'massgis:OWNER_ABRV^M203'
#280 - 9 'shop^bicycle'
#281 - 9 'massgis:FEE_OWNER^Town of New Marlborough'
#284 - 8 'massgis:SITE_NAME^BASHBISH FALLS STATE PARK'
#287 - 8 'massgis:FEE_OWNER^American Chestnut Nominee Trust'
#292 - 8 'massgis:ATT_DATE^2005/02/22'
#293 - 8 'massgis:OLI_1_TYPE^M'
#294 - 8 'massgis:FEE_OWNER^Massachusetts Audubon Society'
#297 - 8 'type^civil'
#298 - 8 'massgis:ASSESS_MAP^34'
#300 - 8 'massgis:OWNER_ABRV^MAS'
#305 - 8 'shop^optician'
#309 - 8 'number^2'
#314 - 8 'massgis:ATT_DATE^2005/12/15'
#315 - 8 'landuse^construction'
#316 - 8 'massgis:SOURCE_MAP^DCS/BNRC'
#317 - 8 'massgis:ATT_DATE^2006/03/28'
#320 - 8 'massgis:OLI_2_INT^APR'
#322 - 8 'massgis:ATT_DATE^2008/11/21'
#323 - 8 'massgis:ASSESS_SUB^A'
#326 - 8 'massgis:OLI_1_ABRV^DCRS'
#327 - 8 'massgis:OLI_1_ORG^DCR - Division of State Parks and Recreation'
#333 - 8 'shop^discount'
#334 - 8 'width^21.3'
#336 - 8 'massgis:ASSESS_LOT^8'
#337 - 8 'wood^mixed'
#338 - 8 'construction^motorway_link'
#339 - 8 'width^45.7'
#342 - 8 'amenity^nightclub'
#345 - 8 'massgis:ASSESS_SUB^PARTIAL'
#348 - 8 'massgis:ALT_SITE_N^SLT/ LARKIN'
#349 - 8 'massgis:ATT_DATE^2008/09/05'
#357 - 7 'massgis:SOURCE_MAP^586'
#358 - 7 'massgis:FEE_OWNER^DOMBROWSKI'
#360 - 7 'width^12.5'
#361 - 7 'massgis:SITE_NAME^JUG END STATE RESERVATION & WMA'
#362 - 7 'massgis:ASSESS_LOT^11'
#363 - 7 'shop^alcohol'
#364 - 7 'massgis:ASSESS_MAP^19'
#365 - 7 'massgis:ALT_SITE_N^CH61B'
#366 - 7 'massgis:ASSESS_LOT^12'
#367 - 7 'massgis:MANAGER^Town of Great Barrington Parks and Recreation Department'
#368 - 7 'massgis:MANAGR_ABR^M113PR'
#369 - 7 'massgis:FEE_OWNER^Haas, John C. & Chara C.'
#370 - 7 'width^6.4'
#371 - 7 'massgis:ASSESS_MAP^30'
#372 - 7 'massgis:BOND_ACCT^21219884'
#373 - 7 'water^yes'
#374 - 7 'massgis:PROJ_ID2^DCS-CR6'
#375 - 7 'massgis:OLI_1_ORG^Richmond Land Trust'
#376 - 7 'construction^yes'
#377 - 7 'name:el^ΟΔΟΣ ΝΤΑΝΦΟΡΘ'
#378 - 7 'massgis:OLI_1_ABRV^RMLT'
#379 - 7 'massgis:ASSESS_MAP^14'
#380 - 7 'massgis:ASSESS_LOT^15'
#381 - 7 'massgis:POLY_ID^1'
#382 - 7 'massgis:ASSESS_MAP^403'
#383 - 7 'massgis:OLI_2_ORG^Beverly Conservation Land Trust'
#384 - 7 'massgis:OLI_2_ABRV^BCLT'
#385 - 7 'flashing_light^nesw'
#386 - 7 'massgis:SITE_NAME^BARTHOLOMEWS COBBLE'
#387 - 7 'highway^services'
#388 - 7 'massgis:COMMENTS^SOURCE FROM TTOR DIGITAL DATA.'
#389 - 7 'massgis:OWNER_ABRV^M267'
#390 - 7 'massgis:FEE_OWNER^Town of Sheffield'
#391 - 7 'massgis:OWNER_ABRV^ACNT'
#392 - 7 'massgis:ATT_DATE^2008/11/05'
#393 - 7 'shop^laundry'
#394 - 7 'massgis:SITE_NAME^LAUREL HILL'
#395 - 7 'massgis:OWNER_ABRV^LHA'
#396 - 7 'maxwidth^2.0m'
#398 - 7 'massgis:FEE_OWNER^Laurel Hill Association'
#400 - 6 'massgis:SITE_NAME^BALDWIN HILL'
#401 - 6 'massgis:ALT_SITE_N^BNRC/ BALDWIN HILL'
#402 - 6 'massgis:FEE_OWNER^TURNER LAND NOMINEE REALTY TRUST'
#403 - 6 'massgis:ATT_DATE^1998/03/30'
#408 - 6 'massgis:ASSESS_MAP^17'
#410 - 6 'massgis:ASSESS_MAP^13'
#411 - 6 'massgis:ATT_DATE^2008/12/11'
#412 - 6 'status^abandoned'
#413 - 6 'massgis:SOURCE_TYP^RPA'
#415 - 6 'name:el^ΟΔΟΣ ΠΑΗΠ'
#416 - 6 'massgis:SITE_NAME^VANDERSMISSEN MEMORIAL PARK'
#418 - 6 'massgis:ASSESS_LOT^6A-6E'
#419 - 6 'massgis:FEE_OWNER^Vandersmissionm Memorial Trust'
#420 - 6 'massgis:SITE_NAME^Sheffield Land Trust/Cavalier'
#421 - 6 'massgis:ATT_DATE^1995/04/03'
#422 - 6 'massgis:ATT_DATE^2006/01/06'
#423 - 6 'maxheight^4.0m'
#424 - 6 'landuse^military'
#425 - 6 'massgis:FEE_OWNER^Sheffield Land Trust'
#426 - 6 'massgis:ALT_SITE_N^FENN'
#427 - 6 'massgis:SITE_NAME^WOODBURN FARM'
#428 - 6 'massgis:OWNER_ABRV^SFLT'
#429 - 6 'massgis:ATT_DATE^1995/04/05'
#432 - 6 'massgis:ATT_DATE^2008/08/25'
#434 - 6 'massgis:COMMENTS^CONTACT THE NATURE CONSERVANCY FOR INFO'
#435 - 6 'sidewalk^both'
#437 - 6 'exit^619'
#438 - 6 'massgis:SITE_NAME^BOSQUET SKI AND TENNIS'
#440 - 6 'massgis:FEE_OWNER^TAMARACK SKI NOMINEE TRUST'
#441 - 6 'massgis:SITE_NAME^Sheffield Land Trust/Chapin'
#442 - 6 'college^Trinity College'
#445 - 6 'massgis:FEE_OWNER^Chapin'
#447 - 6 'width^36.6'
#448 - 6 'width^35.1'
#449 - 6 'width^14.3'
#451 - 6 'amenity^drinking_water'
#453 - 6 'source:ref^wikipedia'
#457 - 6 'massgis:SOURCE_MAP^TTOR'
#458 - 6 'massgis:SITE_NAME^MT EVERETT STATE RES'
#459 - 6 'massgis:SOURCE_MAP^BRPC'
#460 - 6 'massgis:OLI_2_TYPE^M'
#461 - 6 'shop^furniture'
#462 - 6 'massgis:BOND_ACCT^21218882'
#463 - 6 'massgis:OWNER_ABRV^BSO'
#464 - 6 'massgis:ATT_DATE^2003/10/01'
#465 - 6 'massgis:FEE_OWNER^Boston Symphony Orchestra'
#466 - 6 'massgis:OLI_1_ORG^The Trustees of Reservations'
#467 - 6 'barrier^bollard'
#468 - 6 'maxheight^3.9m'
#469 - 6 'massgis:OLI_1_ABRV^TTOR'
#470 - 6 'massgis:FEE_OWNER^Cavalier'
#471 - 6 'massgis:ASSESS_MAP^6'
Sorting Unknown Render entries #12
#0 - 31 'aerialway^unknown'
#1 - 25 'railway^unknown'
#2 - 12 'railway^monorail'
#3 - 7 'oneway^yes'
Freeing Sections!

#7 - 19 'service^spur'
#10 - 13 'bus_schedule^yes'
#15 - 12 'waterway^lock_gate'
#17 - 11 'name_base_1^Broken Point'
#18 - 11 'cutting^yes'
#22 - 9 'bridge^flyover'
#23 - 9 'shop^bicycle'
#24 - 9 'crossing^island'
#26 - 8 'disused^yes'
#28 - 7 'atm^yes'
#30 - 7 'name_direction_prefix^E'
#31 - 7 'zip_left_3^98250'
#32 - 7 'school_zone^yes'
#34 - 7 'zip_right_2^98250'
#35 - 6 'layer^-.5'
#36 - 6 'highway^parking_isle'
#37 - 6 'name_base_1^3 Corner Lake'
#39 - 6 'traffic_calming^hump'
#40 - 6 'capacity^4'
Sorting Unknown Render entries #8
#1 - 22 'highway^crossing'
#2 - 18 'railway^unknown'
#3 - 2 'junction^roundabout'


#22 - 18 'width^2'
#24 - 18 'rel^1'
#26 - 17 'shop^beverages'
#30 - 16 'amenity^drinking_water'
#31 - 15 'route^ski'
#32 - 15 'aerialway^unknown'
#33 - 15 'natural^cliff'
#34 - 13 'speedlimit^50'
#35 - 13 'amenity^car_rental'
#36 - 13 'aeroway^helipad'
#37 - 13 'abutters^residential'
#38 - 13 'abutters^industrial'
#39 - 13 'bus_schedule^yes'
#40 - 13 'motorcar^private'
#41 - 12 'amenity^atm'
#42 - 12 'bikepa_ref^Y'
#44 - 12 'sport^athletics'
#45 - 12 'leisure^nature_preserve'
#46 - 12 'shop^bicycle'
#47 - 12 'type^civil'
#48 - 12 'proposed^primary'
#50 - 11 'building^hall'
#52 - 11 'width^narrow'
#53 - 11 'place^locality'
#54 - 11 'boundary^civil'
#55 - 11 'service^spur'
#56 - 11 'power_source^hydro'
#57 - 11 'sport^skating'
#59 - 11 'disused^yes'
#60 - 10 'bikePA_ref^J'
#61 - 10 'surface^0'
#62 - 10 'piste:difficulty^intermediate'
#63 - 10 'amenity^retirement_home'
#64 - 10 'proposed^trunk_link'
#65 - 10 'amenity^arts_centre'
#66 - 10 'maxweight^10t'
#67 - 10 'sport^skateboard'
#68 - 10 'landuse^construction'
#69 - 10 'shop^outdoor'
#70 - 9 'shop^car_repair'
#71 - 9 'crossing^island'
#72 - 9 'direction^North'
#73 - 9 'proposed^tertiary'
#74 - 9 'shop^laundry'
#75 - 9 'amenity^shelter'
#76 - 9 'direction^Northeast'
#77 - 9 'sport^multi'
#78 - 9 'shop^video_rental'
#79 - 9 'traffic_calming^hump'
#80 - 8 'shop^bakery'
#81 - 8 'class^Secondary'
#82 - 8 'rel^3'
#83 - 8 'landuse^brownfield'
#84 - 8 'motorcycle^no'
#85 - 8 'waterway^lock_gate'
#86 - 8 'historic^memorial'
#87 - 8 'abutters^mixed'
#88 - 8 'landuse^military'
#89 - 8 'proposed^secondary'
#90 - 8 'class^highway'
#91 - 8 'service^alley'
#92 - 7 'shop^books'
#93 - 7 'direction^South'
#94 - 7 'railway^construction'
#95 - 7 'nickname^Duff''s Ditch'
#96 - 7 'school_zone^yes'
#97 - 7 'atm^yes'
#98 - 7 'class^trunk'
#99 - 7 'piste:difficulty^advanced'
#100 - 7 'width^45'
#101 - 7 'railway^preserved'
#102 - 7 'highway^private'
#103 - 7 'line^Barrie Line'
#104 - 7 'amenity^bicycle parking'
#105 - 7 'shop^discount'
#106 - 7 'shop^furniture'
#107 - 7 'waterway^floodway'
#108 - 7 'sport^skiing'
#109 - 6 'aerialway^cable_car'
#110 - 6 'exit^619'
#111 - 6 'sport^curling'
#112 - 6 'name:el^ΟΔΟΣ ΠΑΗΠ'
#113 - 6 'status^abandoned'
#114 - 6 'maxwidth^2.0m'
#115 - 6 'construction^tertiary'
#116 - 6 'flashing_light^nesw'
#117 - 6 'shop^clothing'
#118 - 6 'railway^monorail'
#119 - 6 'aeroway^airport'
#120 - 6 'maxheight^4.0m'
#121 - 6 'embankment^true'
#122 - 6 'highway^ford'
#123 - 6 'piste:difficulty^easy'
#124 - 6 'number^2'
#126 - 6 'cutting^false'
#127 - 6 'construction^yes'



#2 - 65 'aerialway^unknown'
#8 - 17 'railway^construction'
#9 - 13 'highway^osmr:parking'
#10 - 12 'abutters^osmr:parking'
#12 - 11 'shop^electronics'
#14 - 10 'power^wind_farm'
#15 - 10 'amenity^atm'
#16 - 9 'railway^monorail'
#18 - 9 'atm^yes'
#19 - 9 'tourism^picnic_area'
#20 - 9 'landuse^allotments'
#21 - 8 'building^store'
#22 - 8 'border^fence'
#23 - 8 'source:name^wikipedia'
#24 - 8 'disused^yes'
#26 - 7 'abutters^commercial'
#27 - 7 'railway^unknown'
#28 - 7 'man_made^reservoir_covered'
#29 - 7 'railway^abandonded'
#31 - 6 'type^civil'
#32 - 6 'highway^u'
#33 - 6 'aeroway^helipad'
#34 - 6 'dispensing^yes'
#35 - 6 'rollerblade^yes'
#endif

static const OSMTAG_DEF osmtagpairlist[]={
	{ACCESS_FALSE,						"access^false"},
	{ACCESS_FALSE,						"access^no"},
	{ACCESS_TRUE,						"access^yes"},
	{ACCESS_TRUE,						"access^true"},
	{ACCESS_PERMISSIVE,					"access^permissive"},
	{ACCESS_PERMISSIVE,					"access^permisive"},
	{ACCESS_PRIVATE,					"access^private"},
	{ACCESS_PRIVATE,					"private^yes"},
	{ACCESS_RESTRICTED,					"access^restricted"},
	{AEROWAY_AERODROME,					"aeroway^aerodrome"},
	{AEROWAY_AERODROME,					"aeroway^terminal"},
	{AEROWAY_AERODROME,					"building^terminal"},
	{AEROWAY_APRON,						"aeroway^apron"},
	{AEROWAY_TAXIWAY,					"aeroway^taxiway"},
	{AEROWAY_TAXIWAY,					"aerodrome^taxiway"},
	{AEROWAY_RUNWAY,					"aeroway^runway"},
	{AERIALWAY_UNKNOWN,					"aerialway^unknown"},
	{AGRICULTURAL_TRUE,					"agricultural^yes"},
	{AGRICULTURAL_TRUE,					"agricultural^true"},
	{AMENITY_AIRPORTTERMINAL,			"amenity^airport terminal"},
	{AMENITY_AMBULANCESTATION,			"amenity^ambulance station"},
	{AMENITY_AMBULANCESTATION,			"amenity^ambulance_station"},
	{AMENITY_ARTGALLERY,				"amenity^art gallery"},
	{AMENITY_ARTGALLERY,				"amenity^art_gallery"},
	{AMENITY_ATTRACTION,				"amenity^attraction"},
	{AMENITY_ATTRACTION,				"tourism^attraction"},
	{AMENITY_BANK,						"amenity^bank"},
	{AMENITY_BENCH,						"natural^bench"},
	{AMENITY_BICYCLEPARKING,			"amenity^bicycle_parking"},
	{AMENITY_BOATLAUNCH,				"amenity^boat launch"},
	{AMENITY_BUILDING,					"amenity^building"},
	{AMENITY_BUSSTATION,				"amenity^bus station"},
	{AMENITY_BUSSTATION,				"amenity^bus_station"},
	{AMENITY_CAFE,						"amenity^cafe"},
	{AMENITY_CAMPSITE,					"tourism^campsite"},
	{AMENITY_CAMPSITE,					"tourism^camp_site"},
	{AMENITY_CAMPSITE,					"tourism^caravan_site"},
	{AMENITY_CARSHOP,					"shop^car"},
	{AMENITY_CASINO,					"amenity^casino"},
	{AMENITY_CEMETERY,					"landuse^cemetery"},
	{AMENITY_CEMETERY,					"landuse^cemetary"},
	{AMENITY_CEMETERY,					"amenity^grave_yard"},
	{AMENITY_CHAIRLIFT,					"aerialway^chair_lift"},
	{AMENITY_CHURCH,					"amenity^church"},
	{AMENITY_CHURCH,					"amenity^place_of_worship"},
	{AMENITY_CHURCH,					"amenity^place-of-worship"},
	{AMENITY_CHURCH,					"building^church"},
	{AMENITY_COFFEESHOP,				"amenity^coffee"},
	{AMENITY_COFFEESHOP,				"shop^coffee"},
	{AMENITY_COLLEGE,					"amenity^college"},
	{AMENITY_COMMERCIAL,				"landuse^commercial"},
	{AMENITY_COMMUNITYCENTER,			"amenity^community center"},
	{AMENITY_COMMUNITYCENTER,			"amenity^community_centre"},
	{AMENITY_COMMUNITYCENTER,			"amenity^community_hall"},
	{AMENITY_COMMUNITYCENTER,			"amenity^community hall"},
	{AMENITY_CONVENIENCE,				"amenity^convenience"},
	{AMENITY_CONVENIENCE,				"shop^convenience"},
	{AMENITY_COURTHOUSE,				"amenity^court house"},
	{AMENITY_COURTHOUSE,				"amenity^courthouse"},
	{AMENITY_CULTURALCENTER,			"amenity^cultural center"},
	{AMENITY_CULTURALCENTER,			"amenity^cultural centre"},
	{AMENITY_DAM,						"waterway^dam"},
	{AMENITY_DIYSHOP,					"shop^doityourself"},
	{AMENITY_DOCK,						"waterway^dock"},
	{AMENITY_DAYCARE,					"amenity^child_care"},
	{AMENITY_DAYCARE,					"amenity^daycare"},
	{AMENITY_FARM,						"landuse^farm"},
	{AMENITY_FASTFOOD,					"amenity^fast_food"},
	{AMENITY_FERRYTERMINAL,				"amenity^ferry terminal"},
	{AMENITY_FIRESTATION,				"amenity^fire station"},
	{AMENITY_FIRESTATION,				"amenity^fire_station"},
	{AMENITY_FIRESTATION,				"building^fire_station"},
	{AMENITY_FOUNTAIN,					"amenity^fountain"},
	{AMENITY_FUEL,						"amenity^fuel"},
	{AMENITY_FUNERALHOME,				"amenity^funeralhome"},
	{AMENITY_GARBAGEDUMP,				"amenity^garbage"},
	{AMENITY_GARBAGEDUMP,				"amenity^garbagedump"},
	{AMENITY_GARBAGEDUMP,				"amenity^garbage dump"},
	{AMENITY_GARDEN,					"leisure^garden"},
	{AMENITY_GOLFCOURSE,				"leisure^golf_course"},
	{AMENITY_GROCERY,					"amenity^grocery"},
	{AMENITY_HANGAR,					"building^hangar"},
	{AMENITY_HOCKEYRINK,				"amenity^hockeyrink"},
	{AMENITY_HOCKEYRINK,				"sport^hockey"},
	{AMENITY_HOSPITAL,					"amenity^doctor"},
	{AMENITY_HOSPITAL,					"amenity^hospital"},
	{AMENITY_HOSPITAL,					"amenity^clinic"},
	{AMENITY_HOSPITAL,					"amenity^medical clinic"},
	{AMENITY_HOSPITAL,					"building^hospital"},
	{AMENITY_HOTEL,						"amenity^hotel"},
	{AMENITY_HOTEL,						"building^hotel"},
	{AMENITY_HOTEL,						"tourism^hotel"},
	{AMENITY_HOTEL,						"tourism^hostel"},
	{AMENITY_INDUSTRIAL,				"landuse^industrial"},
	{AMENITY_INFORMATION,				"tourism^information"},
	{AMENITY_LANDFILL,					"landuse^landfill"},
	{AMENITY_LIBRARY,					"amenity^library"},
	{AMENITY_MAILBOX,					"amenity^post_box"},
	{AMENITY_MAILBOX,					"post_box^community_mailbox"},
	{AMENITY_MARINA,					"leisure^marina"},
	{AMENITY_MONUMENT,					"historic^monument"},
	{AMENITY_MOTEL,						"building^motel"},
	{AMENITY_MOTEL,						"tourism^motel"},
	{AMENITY_MUSEUM,					"amenity^museum"},
	{AMENITY_MUSEUM,					"amenity^muesum"},
	{AMENITY_MUSEUM,					"building^museum"},
	{AMENITY_MUSEUM,					"tourism^museum"},
	{AMENITY_MUSEUM,					"tourism^artwork"},
	{AMENITY_OFFICE,					"amenity^office"},
	{AMENITY_PARKING,					"amenity^parking"},
	{AMENITY_PARKING,					"parking^surface"},
	{AMENITY_PARKING,					"service^parking_aisle"},
	{AMENITY_PARKING,					"parking^multi-storey"},
	{AMENITY_PHARMACY,					"amenity^pharmacy"},
	{AMENITY_PIPELINE,					"man_made^pipeline"},
	{AMENITY_PIER,						"man_made^pier"},
	{AMENITY_PLAYGROUND,				"leisure^playground"},
	{AMENITY_PLAYGROUND,				"leisure^playground"},
	{AMENITY_POLICE,					"amenity^police"},
	{AMENITY_POLICE,					"amenity^police station"},
	{AMENITY_POSTOFFICE,				"amenity^post office"},
	{AMENITY_POSTOFFICE,				"amenity^post_office"},
	{AMENITY_PRISON,					"amenity^prison"},
	{AMENITY_PUB,						"amenity^pub"},
	{AMENITY_QUARRY,					"landuse^quarry"},
	{AMENITY_PUBLICBUILDING,			"amenity^public_building"},
	{AMENITY_RECREATIONCENTER,			"amenity^recreation center"},
	{AMENITY_RECREATIONCENTER,			"building^recreation_center"},
	{AMENITY_RECREATIONCENTER,			"leisure^sports_centre"},
	{AMENITY_RECREATIONCENTER,			"leisure^arena"},
	{AMENITY_RECREATIONCENTER,			"leisure^common"},
	{AMENITY_RECYCLING,					"amenity^recycling"},
	{AMENITY_RESIDENTIAL,				"landuse^residential"},
	{AMENITY_RESIDENTIAL,				"building^residential"},
	{AMENITY_RESIDENTIAL,				"building^apartments"},
	{AMENITY_RESTRAUNT,					"amenity^restaurant"},
	{AMENITY_RETAIL,					"landuse^retail"},
	{AMENITY_SCHOOL,					"amenity^school"},
	{AMENITY_SCHOOL,					"type^school"},
	{AMENITY_SCHOOL,					"building^school"},
	{AMENITY_SCHOOL,					"landuse^school"},
	{AMENITY_SERVICEYARD,				"service^yard"},
	{AMENITY_SHOPPING,					"amenity^shop"},
	{AMENITY_SHOPPING,					"amenity^shopping"},
	{AMENITY_SHOPPING,					"amenity^shopping mall"},
	{AMENITY_SHOPPING,					"shop^department_store"},
	{AMENITY_SHOPPING,					"building^mall"},
	{AMENITY_SHOPPING,					"shop^mall"},
	{AMENITY_SHOPPING,					"shop^shopping_center"},
	{AMENITY_STADIUM,					"amenity^stadium"},
	{AMENITY_STADIUM,					"building^stadium"},
	{AMENITY_STADIUM,					"leisure^stadium"},
	{AMENITY_SUPERMARKET,				"amenity^supermarket"},
	{AMENITY_SUPERMARKET,				"building^supermarket"},
	{AMENITY_SUPERMARKET,				"shop^supermarket"},
	{AMENITY_SWIMMINGPOOL,				"amenity^swimming pool"},
	{AMENITY_SWIMMINGPOOL,				"leisure^swimming_pool"},
	{AMENITY_TELEPHONE,					"amenity^telephone"},
	{AMENITY_TENNISCOURT,				"amenity^tennis court"},
	{AMENITY_TENNISCOURT,				"amenity^tennis"},
	{AMENITY_TENNISCOURT,				"leisure^tennis_court"},
	{AMENITY_THEATRE,					"amenity^theatre"},
	{AMENITY_THEATRE,					"amenity^cinema"},
	{AMENITY_TOILETS,					"amenity^toilets"},
	{AMENITY_TOWNHALL,					"amenity^townhall"},
	{AMENITY_TOWNHALL,					"amenity^town_hall"},
	{AMENITY_TOWNHALL,					"amenity^city hall"},
	{AMENITY_TOWNHALL,					"building^municipal"},
	{AMENITY_TOWER,						"amenity^tower"},
	{AMENITY_TOWER,						"building^tower"},
	{AMENITY_TOWER,						"man_made^tower"},
	{AMENITY_SPORTSTRACK,				"leisure^track"},
	{AMENITY_TRAINSTATION,				"amenity^train station"},
	{AMENITY_TRAINSTATION,				"building^train station"},
	{AMENITY_UNDERGROUNDPARKING,		"parking^underground"},
	{AMENITY_UNIVERSITY,				"amenity^university"},
	{AMENITY_UNIVERSITY,				"building^university"},
	{AMENITY_UNIVERSITY,				"learning^university"},
	{AMENITY_VIEWPOINT,					"tourism^viewpoint"},
	{AMENITY_WATERPARK,					"leisure^water_park"},
	{AMENITY_WATERTOWER,				"man_made^water_tower"},
	{AMENITY_ZOO,					"landuse^zoo"},
	{AMENITY_ZOO,					"tourism^zoo"},
	{AREA_TRUE,						"area^yes"},
	{AREA_TRUE,						"area^true"},
	{AREA_FALSE,					"area^no"},
	{AREA_FALSE,					"area^false"},
	{BICYCLE_TRUE,					"bike^yes"},
	{BICYCLE_TRUE,					"bicycle^yes"},
	{BICYCLE_TRUE,					"bicycle^designated"},
	{BICYCLE_FALSE,					"bicycle^no"},
	{BOAT_TRUE,						"boat^yes"},
	{BOAT_TRUE,						"boat^true"},
	{BOAT_FALSE,					"boat^no"},
	{BOAT_FALSE,					"boat^false"},
	{BORDER_COUNTY,					"border_type^county"},
	{BORDER_PROVINCE,				"border_type^province"},
	{BORDER_PROVINCE,				"border_type^state"},
	{BORDER_COUNTRY,				"border_type^country"},
	{BOUNDARY_ADMINISTRATIVE,		"boundary^administrative"},
	{BRIDGE_TRUE,					"bridge^true"},
	{BRIDGE_TRUE,					"bridge^1"},
	{BRIDGE_TRUE,					"bridge^yes"},
	{BRIDGE_FALSE,					"bridge^false"},
	{BRIDGE_FALSE,					"bridge^no"},
	{BRIDGE_FALSE,					"bridge^0"},
	{BUILDING_TRUE,					"building^yes"},
	{BUILDING_TRUE,					"building^true"},
	{BUILDING_TRUE,					"building^1"},
	{BUILDING_TRUE,					"building^block"},
	{BUS_TRUE,						"bus^yes"},
	{BUS_TRUE,						"bus^true"},
	{BUS_FALSE,						"bus^false"},
	{BUS_FALSE,						"bus^no"},
	{CAR_TRUE,						"motorcar^yes"},
	{CAR_TRUE,						"motorcar^true"},
	{CAR_FALSE,						"motorcar^false"},
	{CAR_FALSE,						"motorcar^no"},
	{CLASS_MINOR,					"class^minor"},
	{CLASS_PRIMARY,					"class^primary"},
	{CLASS_SECONDARY,				"class^secondary"},
	{CONSTRUCTION_PRIMARY,			"construction^primary"},
	{CONSTRUCTION_RESIDENTIAL,		"construction^residential"},
	{CONDITION_DEFICIENT,			"condition^deficient"},
	{CONDITION_FAIR,				"condition^fair"},
	{CONDITION_GOOD,				"condition^good"},
	{CONDITION_INTOLERABLE,			"condition^intolerable"},
	{CROSSING_TRAFFIC_SIGNALS,		"crossing^traffic_signals"},
	{CROSSING_UNCONTROLLED,			"crossing^uncontrolled"},
	{CYCLEWAY_LANE,					"cycleway^lane"},
	{CYCLEWAY_TRACK,				"cycleway^track"},
	{CYCLEWAY_TRUE,					"cycleway^true"},
	{CYCLEWAY_TRUE,					"cycleway^yes"},
	{DENOMINATION_ADVENTIST,		"denomination^adventist"},
	{DENOMINATION_ALLIANCE,			"denomination^alliance"},
	{DENOMINATION_ANGLICAN,			"denomination^anglican"},
	{DENOMINATION_BAHAI,			"denomination^baha'i"},
	{DENOMINATION_BAPTIST,			"denomination^baptist"},
	{DENOMINATION_BUDDHIST,			"denomination^buddhist"},
	{DENOMINATION_BUDDHIST,			"religion^buddhist"},
	{DENOMINATION_CANADIANREFORMED,	"denomination^canadian reformed"},
	{DENOMINATION_CATHOLIC,			"denomination^catholic"},
	{DENOMINATION_CATHOLIC,			"religion^catholic"},
	{DENOMINATION_CHRISTIAN,		"denomination^christian"},
	{DENOMINATION_CHRISTIAN,		"religion^christian"},
	{DENOMINATION_CHRISTIANSCIENCE,	"denomination^christian science"},
	{DENOMINATION_CHRISTIANSCIENCE,	"denomination^christian scientist"},
	{DENOMINATION_COPTIC,			"religion^coptic"},
	{DENOMINATION_EVANGELICAL,		"denomination^evangelical"},
	{DENOMINATION_EVANGELICALFREE,	"denomination^evangelical free"},
	{DENOMINATION_GOSPEL,			"denomination^gospel"},
	{DENOMINATION_GREEKORTHODOX,	"denomination^greek orthodox"},
	{DENOMINATION_GREEKORTHODOX,	"denomination^greek_orthodox"},
	{DENOMINATION_INDEPENDENT,		"denomination^independent"},
	{DENOMINATION_ISLAM,			"religion^islam"},
	{DENOMINATION_JEHOVAHSWITNESS,	"denomination^jehovahswitness"},
	{DENOMINATION_JEHOVAHSWITNESS,	"denomination^jehovahs_witness"},
	{DENOMINATION_JEHOVAHSWITNESS,	"denomination^jehovah's witness"},
	{DENOMINATION_JEWISH,			"denomination^jewish"},
	{DENOMINATION_JEWISH,			"religion^jewish"},
	{DENOMINATION_LUTHERAN,			"denomination^lutheran"},
	{DENOMINATION_LUTHERAN,			"denomination^luthern"},
	{DENOMINATION_MENNONITE,		"denomination^mennonite"},
	{DENOMINATION_METHODIST,		"denomination^methodist"},
	{DENOMINATION_MORMON,			"denomination^mormon"},
	{DENOMINATION_NORTHAMERICANBAPTIST,	"denomination^north american baptist"},
	{DENOMINATION_ORTHODOX,			"denomination^orthodox"},
	{DENOMINATION_PENTECOSTAL,		"denomination^pentecostal"},
	{DENOMINATION_PRESBYTERIAN,		"denomination^presbyterian"},
	{DENOMINATION_PROTESTANT,		"denomination^protestant"},
	{DENOMINATION_ROMANCATHOLIC,		"denomination^roman catholic"},
	{DENOMINATION_RUSSIANORTHODOX,		"denomination^russian_orthodox"},
	{DENOMINATION_SALVATIONARMY,		"denomination^salvation_army"},
	{DENOMINATION_SEVENTHDAYADVENTIST,	"denomination^seventh day adventist"},
	{DENOMINATION_UKRANIANBAPTIST,		"denomination^ukranian_baptist"},
	{DENOMINATION_UKRANIANCATHOLIC,		"denomination^ukranian catholic"},
	{DENOMINATION_UKRANIANORTHODOX,		"denomination^ukranian_orthodox"},
	{DENOMINATION_UNITED,				"denomination^united"},
	{DENOMINATION_UNITED,				"denomination^united church"},
	{DENOMINATION_WESLEYAN,				"denomination^wesleyan"},
	{DIRECTION_BACKWARD,				"direction^backward"},
	{DIRECTION_FORWARD,					"direction^forward"},
	{DIRECTION_NORTHEAST,				"direction^north east"},
	{DIRECTION_SOUTHEAST,				"direction^south east"},
	{DIRECTION_NORTHWEST,				"direction^north west"},
	{DIRECTION_SOUTHWEST,				"direction^south west"},
	{DISTANCE_MARKER_TRUE,				"distance_marker^yes"},
	{DISTANCE_MARKER_TRUE,				"distance_marker^true"},
	{DISPENSING_TRUE,					"dispensing^yes"},
	{DISPENSING_TRUE,					"dispensing^true"},
	{EMERGENCY_TRUE,					"emergency^yes"},
	{EMERGENCY_TRUE,					"emergency^true"},
	{EMERGENCY_FALSE,					"emergency^false"},
	{EMERGENCY_FALSE,					"emergency^no"},
	{FEE_TRUE,							"fee^true"},
	{FEE_TRUE,							"fee^yes"},
	{FEE_FALSE,							"fee^no"},
	{FEE_FALSE,							"fee^false"},
	{FOOT_TRUE,							"foot^true"},
	{FOOT_TRUE,							"foot^yes"},
	{FOOT_FALSE,						"foot^no"},
	{FOOT_FALSE,						"foot^false"},
	{FOOT_DESIGNATED,					"foot^designated"},
	{FOOT_PERMISSIVE,					"foot^permissive"},
	{GOODS_TRUE,						"goods^true"},
	{GOODS_TRUE,						"goods^yes"},
	{GOODS_FALSE,						"goods^no"},
	{GOODS_FALSE,						"goods^false"},
	{HIGHWAY_BRIDLEWAY,			"highway^bridleway"},
	{HIGHWAY_BRIDGE,			"highway^bridge"},
	{HIGHWAY_BUS_STOP,			"highway^bus_stop"},
	{HIGHWAY_CATTLEGRID,		"highway^cattle_grid"},
	{HIGHWAY_CONSTRUCTION,		"highway^construction"},
	{HIGHWAY_CROSSING,			"highway^crossing"},
	{HIGHWAY_FOOTWAY,			"highway^footway"},
	{HIGHWAY_FOOTWAY,			"highway^footpath"},
	{HIGHWAY_GATE,				"highway^gate"},
	{HIGHWAY_MINI_ROUNDABOUT,	"highway^mini_roundabout"},
	{HIGHWAY_MINOR,				"highway^minor"},
	{HIGHWAY_MOTORWAY,			"highway^motorway"},
	{HIGHWAY_MOTORWAY_LINK,		"highway^motorway_link"},
	{HIGHWAY_MOTORWAYJUNCTION,	"highway^motorway_junction"},
	{HIGHWAY_PROPOSED,			"highway^proposed"},
	{HIGHWAY_TRUNK,				"highway^trunk"},
	{HIGHWAY_TRUNK_LINK,		"highway^trunk_link"},
	{HIGHWAY_TURNING_CIRCLE,	"highway^turning_circle"},
	{HIGHWAY_PATH,				"highway^path"},
	{HIGHWAY_PEDESTRIAN,		"highway^pedestrian"},
	{HIGHWAY_PRIMARY,			"highway^primary"},
	{HIGHWAY_PRIMARYLINK,		"highway^primary_link"},
	{HIGHWAY_ROAD,				"highway^road"},
	{HIGHWAY_ROAD,				"leisure^slipway"},
	{HIGHWAY_SECONDARY,			"highway^secondary"},
	{HIGHWAY_SECONDARYLINK,		"highway^secondary_link"},
	{HIGHWAY_SPEEDBUMP,			"highway^speed_bump"},
	{HIGHWAY_SPEEDBUMP,			"traffic_calming^speed_bump"},
	{HIGHWAY_SPEEDBUMP,			"traffic_calming^speed_hump"},
	{HIGHWAY_SPEEDBUMP,			"traffic_calming^hump"},
	{HIGHWAY_SPEEDBUMP,			"traffic_calming^bump"},
	{HIGHWAY_STEPS,				"highway^steps"},
	{HIGHWAY_STOP,				"highway^stop"},
	{HIGHWAY_TERTIARY,			"highway^tertiary"},
	{HIGHWAY_TOLLBOOTH,			"highway^toll booth"},
	{HIGHWAY_TOLLBOOTH,			"highway^toll_booth"},
	{HIGHWAY_TOLLBOOTH,			"highway^tollbooth"},
	{HIGHWAY_TRACK,				"highway^track"},
	{HIGHWAY_TRAILHEAD,			"highway^trailhead"},
	{HIGHWAY_RESIDENTIAL,		"highway^residential"},
	{HIGHWAY_CYCLEWAY,			"highway^cycleway"},
	{HIGHWAY_SERVICE,			"highway^service"},
	{HIGHWAY_UNCLASSIFIED,		"highway^unclassified"},
	{HIGHWAY_UNPAVED,			"paved^false"},
	{HIGHWAY_UNPAVED,			"paved^no"},
	{HIGHWAY_UNPAVED,			"highway^unpaved"},
	{HIGHWAY_UNPAVED,			"highway^unsurfaced"},
	{HORSE_TRUE,				"horse^true"},
	{HORSE_TRUE,				"horse^yes"},
	{HORSE_FALSE,				"horse^false"},
	{HORSE_FALSE,				"horse^no"},
	{HGV_TRUE,					"hgv^true"},
	{HGV_TRUE,					"hgv^yes"},
	{HGV_FALSE,					"hgv^no"},
	{HGV_FALSE,					"hgv^false"},
	{LAYER_N5,					"layer^-5"},
	{LAYER_N4,					"layer^-4"},
	{LAYER_N3,					"layer^-3"},
	{LAYER_N2,					"layer^-2"},
	{LAYER_N1,					"layer^-1"},
	{LAYER_0,					"layer^0"},
	{LAYER_1,					"layer^1"},
	{LAYER_2,					"layer^2"},
	{LAYER_3,					"layer^3"},
	{LAYER_4,					"layer^4"},
	{LAYER_5,					"layer^5"},
	{LCN_TRUE,					"lcn^yes"},
	{LEISURE_BENCH,				"amenity^bench"},
	{LEISURE_PARK,				"amenity^park"},
	{LEISURE_PARK,				"tourism^picnic_site"},
	{LEISURE_PARK,				"leisure^park"},
	{LEISURE_PARK,				"landuse^park"},
	{LEISURE_PARK,				"natural^park"},
	{LEISURE_PARK,				"leisure^nature_reserve"},
	{LEISURE_PARK,				"landuse^conservation"},
	{LEISURE_PARK,				"leisure^nature_preserve"},
	{LEISURE_PITCH,				"leisure^pitch"},
	{LEISURE_PITCH,				"landuse^field"},
	{LEISURE_RECREATIONAREA,	"landuse^recreation_area"},
	{LEISURE_RECREATIONAREA,	"landuse^recreation_ground"},
	{LEISURE_RECREATIONAREA,	"leisure^recreation_ground"},
	{LEVEL_1,					"level^1"},
	{LEVEL_2,					"level^2"},
	{LINK_TRUE,					"link^true"},
	{LINK_TRUE,					"link^yes"},
	{NATURAL_BEACH,				"natural^beach"},
	{NATURAL_CANAL,				"waterway^canal"},
	{NATURAL_CAVE,				"natural^cave"},
	{NATURAL_COASTLINE,			"natural^coastline"},
	{NATURAL_COASTLINE,			"landuse^coastline"},
	{NATURAL_DRAIN,				"waterway^drain"},
	{NATURAL_GRASS,				"landuse^grass"},
	{NATURAL_GRASS,				"natural^grass"},
	{NATURAL_FIELD,				"landuse^greenfield"},
	{NATURAL_FOREST,			"natural^forest"},
	{NATURAL_FOREST,			"landuse^forest"},
	{NATURAL_FOREST,			"natural^wood"},
	{NATURAL_LAND,				"natural^land"},
	{NATURAL_LAND,				"natural^island"},
	{NATURAL_LAND,				"island^yes"},
	{NATURAL_LAND,				"landuse^island"},
	{NATURAL_LAND,				"place^island"},
	{NATURAL_LAKE,				"natural^lake"},
	{NATURAL_LAKE,				"place^lake"},
	{NATURAL_LAKE,				"landuse^manmade_lake"},
	{NATURAL_LAKE,				"waterway^lake"},
	{NATURAL_MARSH,				"natural^marsh"},
	{NATURAL_PEAK,				"natural^peak"},
	{NATURAL_SCRUB,				"natural^scrub"},
	{NATURAL_RESERVOIR,			"landuse^reservoir"},
	{NATURAL_RIVER,				"waterway^river"},
	{NATURAL_RIVERBANK,			"waterway^riverbank"},
	{NATURAL_STREAM,			"waterway^stream"},
	{NATURAL_WATER,				"waterway^intermittent"},
	{NATURAL_WATER,				"natural^water"},
	{NATURAL_WATER,				"landuse^water"},
	{NATURAL_WATERFALL,			"waterway^waterfall"},
	{NCN_TRUE,					"ncn^true"},
	{NCN_TRUE,					"ncn^yes"},
	{NOEXIT_TRUE,				"noexit^yes"},
	{NOEXIT_TRUE,				"noexit^true"},
	{NOEXIT_FALSE,				"noexit^no"},
	{NOEXIT_FALSE,				"noexit^false"},
	{ONEWAY_TRUE,				"oneway^yes"},
	{ONEWAY_TRUE,				"oneway^-1"},
	{ONEWAY_TRUE,				"oneway^true"},
	{ONEWAY_TRUE,				"oneway^1"},
	{ONEWAY_FALSE,				"oneway^0"},
	{ONEWAY_FALSE,				"oneway^no"},
	{ONEWAY_FALSE,				"oneway^false"},
	{ONEWAY_REVERSIBLE,			"oneway^reversible"},
	{PASSENGER_TRUE,			"passenger^true"},
	{PASSENGER_TRUE,			"passenger^yes"},
	{PROPOSED_RESIDENTIAL,		"proposed^residential"},
	{PROPOSED_PRIMARYLINK,		"proposed^primary_link"},
	{LANES_HALF,				"lanes^0.5"},
	{LANES_1,					"lanes^1"},
	{LANES_2,					"lanes^2"},
	{LANES_3,					"lanes^3"},
	{LANES_4,					"lanes^4"},
	{LANES_5,					"lanes^5"},
	{LANES_6,					"lanes^6"},
	{MOTORCYCLE_TRUE,			"motorcycle^true"},
	{MOTORCYCLE_TRUE,			"motorcycle^yes"},
	{MOTORCYCLE_FALSE,			"motorcycle^false"},
	{MOTORCYCLE_FALSE,			"motorcycle^no"},
	{JUNCTION_TRUE,				"junction^true"},
	{JUNCTION_TRUE,				"junction^yes"},
	{JUNCTION_ROUNDABOUT,		"junction^roundabout"},
	{JUNCTION_TRAFFICLIGHT,		"junction^traffic light"},
	{JUNCTION_TRAFFICLIGHT,		"junction^traffic_signal"},
	{PLACE_AIRPORT,				"amenity^airport"},
	{PLACE_AIRPORT,				"place^airport"},
	{PLACE_CITY,				"place^city"},
	{PLACE_COUNTY,				"place^county"},
	{PLACE_HAMLET,				"place^hamlet"},
	{PLACE_SUBURB,				"place^suburb"},
	{PLACE_TOWN,				"place^town"},
	{PLACE_VILLAGE,				"place^village"},
	{POWER_GENERATOR,			"power^generator"},
	{POWER_LINE,				"power^line"},
	{POWER_SUBSTATION,			"power^sub station"},
	{POWER_SUBSTATION,			"power^sub_station"},
	{POWER_SUBSTATION,			"power^substation"},
	{POWER_SUBSTATION,			"man_made^sub station"},
	{POWER_SUBSTATION,			"man_made^sub_station"},
	{POWER_SUBSTATION,			"man_made^substation"},
	{POWER_TOWER,				"power^tower"},
	{POWERSOURCE_WIND,			"power_source^wind"},
	{PSV_TRUE,					"psv^true"},
	{PSV_TRUE,					"psv^yes"},
	{PSV_FALSE,					"psv^no"},
	{PSV_FALSE,					"psv^false"},
	{RAILWAY_ABANDONED,			"railway^abandoned"},
	{RAILWAY_ABANDONED,			"railway^disused"},
	{RAILWAY_CROSSING,			"railway^crossing"},
	{RAILWAY_LAND,				"landuse^railway"},
	{RAILWAY_HALT,				"railway^halt"},
	{RAILWAY_LEVELCROSSING,		"railway^level_crossing"},
	{RAILWAY_LIGHTRAIL,			"railway^light_rail"},
	{RAILWAY_MONORAIL,			"railway^monorail"},
	{RAILWAY_PROPOSED,			"railway^proposed"},
	{RAILWAY_RAIL,				"railway^rail"},
	{RAILWAY_RAIL,				"highway^railway"},
	{RAILWAY_SPUR,				"railway^spur"},
	{RAILWAY_STATION,			"railway^station"},
	{RAILWAY_SUBWAY,			"railway^subway"},
	{RAILWAY_SUBWAYENTRANCE,	"railway^subway_entrance"},
	{RAILWAY_TRAM,				"railway^tram"},
	{RAILWAY_UNKNOWN,			"railway^unknown"},
	{RAILWAY_YARD,				"railway^yard"},
	{RCN_TRUE,					"rcn^true"},
	{RCN_TRUE,					"rcn^yes"},
	{ROUTE_BUS,					"route^bus"},
	{ROUTE_FERRY,				"route^ferry"},
	{ROUTE_SKI,					"route^ski"},
	{SEPERATED_TRUE,			"seperated^true"},
	{SEPERATED_TRUE,			"seperated^yes"},
	{SEPERATED_FALSE,			"seperated^false"},
	{SEPERATED_FALSE,			"seperated^no"},
	{SEPERATED_FALSE,			"separated^no"},
	{SHELTER_TRUE,				"shelter^true"},
	{SHELTER_TRUE,				"shelter^yes"},
	{SHELTER_FALSE,				"shelter^false"},
	{SHELTER_FALSE,				"shelter^no"},
	{SIZE_HUGE,					"size^huge"},
	{SIZE_LARGE,				"size^large"},
	{SIZE_MEDIUM,				"size^medium"},
	{SPORT_BASKETBALL,			"sport^basketball"},
	{SPORT_BASEBALL,			"sport^baseball"},
	{SPORT_BASEBALL,			"sport^softball"},
	{SPORT_FOOTBALL,			"sport^football"},
	{AMENITY_GOLFCOURSE,		"sport^golf"},
	{SPORT_SOCCER,				"sport^soccer"},
	{AMENITY_SWIMMINGPOOL,		"sport^swimming"},
	{SPORT_TENNIS,				"sport^tennis"},
	{SURFACE_ASPHALT,			"surface^asphalt"},
	{SURFACE_DIRT,				"surface^dirt"},
	{SURFACE_GRASS,				"surface^grass"},
	{SURFACE_GRAVEL,			"surface^gravel"},
	{SURFACE_PAVED,				"surface^paved"},
	{SURFACE_PAVED,				"paved^yes"},
	{SURFACE_WOOD,				"surface^wood"},
	{SURFACE_WOOD,				"surface^wooden"},
	{SURFACE_UNPAVED,			"surface^unpaved"},
	{TAXI_TRUE,					"taxi^yes"},
	{TAXI_TRUE,					"taxi^true"},
	{TAXI_FALSE,				"taxi^false"},
	{TAXI_FALSE,				"taxi^no"},
	{TOLL_TRUE,					"toll^yes"},
	{TOLL_TRUE,					"toll^true"},
	{TOLL_FALSE,				"toll^false"},
	{TOLL_FALSE,				"toll^no"},
	{TRACKTYPE_GRADE1,			"tracktype^grade1"},
	{TRACKTYPE_GRADE2,			"tracktype^grade2"},
	{TRACKTYPE_GRADE3,			"tracktype^grade3"},
	{TRACKTYPE_GRADE4,			"tracktype^grade4"},
	{TRACKS_2,					"tracks^2"},
	{TRUCKROUTE_TRUE,			"route^truck"},
	{TUNNEL_TRUE,				"tunnel^true"},
	{TUNNEL_TRUE,				"tunnel^yes"},
	{TUNNEL_FALSE,				"tunnel^false"},
	{TUNNEL_FALSE,				"tunnel^no"},
	{TYPE_CMB,					"type^cmb"},
	{WHEELCHAIR_TRUE,			"wheelchair^true"},
	{WHEELCHAIR_TRUE,			"wheelchair^yes"},
	{WHEELCHAIR_FALSE,			"wheelchair^false"},
	{WHEELCHAIR_FALSE,			"wheelchair^no"},
	{HIGHWAY_TRAFFIC_SIGNALS,	"highway^traffic_signals"}};

void OSMConvert::Init(void)
{
	unsigned int i;
	const OSMTAG_DEF *tp;

	m_printcount=0;
	InitRenderLookup();

	m_taghash.Init(16,sizeof(int));
	m_taggroupspecialhash.Init(16,sizeof(int));
	m_tagpairhash.Init(16,sizeof(OSMTAG_DEF *));
	m_unknownpair.Init(16,sizeof(int));
	m_unknownrender.Init(16,sizeof(int));
	m_nodehash.Init(16,sizeof(OSMCONVNODE_DEF *));

	/* allocate space for node pointers */
	m_tempnodeptrs.Init(8192,-1);
	m_tempnodeptrs2.Init(8192,-1);

	for(i=0;i<sizeof(osmtaglist)/sizeof(OSMTAG_DEF);++i)
		m_taghash.Add(osmtaglist[i].tag,&osmtaglist[i].id);

	for(i=0;i<sizeof(osmtagspecial)/sizeof(OSMTAG_DEF);++i)
		m_taggroupspecialhash.Add(osmtagspecial[i].tag,&osmtagspecial[i].id);

	for(i=0;i<sizeof(osmtagpairlist)/sizeof(OSMTAG_DEF);++i)
	{
		tp=&osmtagpairlist[i];
		m_tagpairhash.Add(osmtagpairlist[i].tag,&tp);
	}
	/* allocate in 1MB chunks */
	m_heap.SetBlockSize(1024*1024);
}

int OSMConvert::SortUnknown(const void *v1,const void *v2)
{
	HashEntry *he1=*((HashEntry **)v1);
	HashEntry *he2=*((HashEntry **)v2);
	int *ip1;
	int *ip2;
	
	ip1=(int *)he1->m_data;
	ip2=(int *)he2->m_data;
	return(ip2[0]-ip1[0]);
}

void OSMConvert::PrintUnknownPairs(void)
{
	unsigned int i,ne;
	HashEntry *he;
	int *ip;
	Array<HashEntry *>helist;
	int hits;
	bool show;
	kGUIString *s;

	ne=m_unknownpair.GetNum();
	if(!ne)
		return;

	helist.Alloc(ne);
	he=m_unknownpair.GetFirst();
	for(i=0;i<ne;++i)
	{
		helist.SetEntry(i,he);
		he=he->GetNext();
	}

	s=new kGUIString();
	s->Sprintf("Sorting Unknown Tags entries #%d\n",ne);
	while(m_comm.Write(&s)==false);

	helist.Sort(ne,SortUnknown);

	//kGUI::Trace("Writing Unknown!\n");
	for(i=0;i<ne;++i)
	{
		//kGUI::Trace("Writing Unknown #%d!\n",i);
		he=helist.GetEntry(i);
		ip=(int *)he->m_data;
		hits=ip[0];
		if(hits>50)
			show=true;
		else if(strncmp(he->m_string,"tiger:",6) && hits>5)
			show=true;
		else
			show=false;

		if(show)
		{
			s=new kGUIString();
			s->Sprintf("#%d - %d '%s'\n",i,hits,he->m_string);
			//kGUI::Trace("Start Comm!\n");
			while(m_comm.Write(&s)==false);
			//kGUI::Trace("End Comm!\n");
		}
	}
}

void OSMConvert::AddUnknownPair(const char *s)
{
	int *ip;
	static const int one=1;

	/* I don't know this tagpair */
	ip=(int *)m_unknownpair.Find(s);
	if(ip)
		ip[0]=ip[0]+1;
	else
		m_unknownpair.Add(s,&one);
}

void OSMConvert::PrintUnknownRender(void)
{
	unsigned int i,ne;
	HashEntry *he;
	int *ip;
	Array<HashEntry *>helist;
	int hits;
	bool show;
	kGUIString *s;

	ne=m_unknownrender.GetNum();
	if(!ne)
		return;

	helist.Alloc(ne);
	he=m_unknownrender.GetFirst();
	for(i=0;i<ne;++i)
	{
		helist.SetEntry(i,he);
		he=he->GetNext();
	}

	s=new kGUIString();
	s->Sprintf("Sorting Unknown Render entries #%d\n",ne);
	while(m_comm.Write(&s)==false);

	helist.Sort(ne,SortUnknown);

	//kGUI::Trace("Writing Unknown!\n");
	for(i=0;i<ne;++i)
	{
		//kGUI::Trace("Writing Unknown #%d!\n",i);
		he=helist.GetEntry(i);
		ip=(int *)he->m_data;
		hits=ip[0];
		if(hits>1)
			show=true;
		else
			show=false;

		if(show)
		{
			s=new kGUIString();
			s->Sprintf("#%d - %d '%s'\n",i,hits,he->m_string);
			//kGUI::Trace("Start Comm!\n");
			while(m_comm.Write(&s)==false);
			//kGUI::Trace("End Comm!\n");
		}
	}
}

void OSMConvert::AddUnknownRender(const char *s)
{
	int *ip;
	static const int one=1;

	/* I don't know this tagpair */
	ip=(int *)m_unknownrender.Find(s);
	if(ip)
		ip[0]=ip[0]+1;
	else
		m_unknownrender.Add(s,&one);
}

#define PRINTCOUNT 10000

void OSMConvert::ChildLoaded(kGUIXMLItem *child,kGUIXMLItem *parent)
{
	int *itag;

	itag=(int *)m_taghash.Find(child->GetName());
	if(itag)
	{
		switch(itag[0])
		{
		case OSMTAG_NODE:
		{
			unsigned int i;
			unsigned int nc;
			OSMCONVNODE_DEF node;
			kGUIXMLItem *parm;
			const char *id=0;
			unsigned int numtags;
			OSMTAG_DEF *tags[MAXTAGS];
			OSMTAG_DEF **ptag;

			numtags=0;
			node.m_name=0;
			/* collect attributes for this node */
			nc=child->GetNumChildren();
			for(i=0;i<nc;++i)
			{
				parm=child->GetChild(i);
				itag=(int *)m_taghash.Find(parm->GetName());
				if(itag)
				{
					switch(itag[0])
					{
					case OSMTAG_TIMESTAMP:
					case OSMTAG_USER:
					case OSMTAG_VISIBLE:
						/* ignore these */
					break;
					case OSMTAG_ID:
						id=parm->GetValueString();
					break;
					case OSMTAG_LAT:
						node.m_lat=parm->GetValueDouble();
					break;
					case OSMTAG_LON:
						node.m_lon=parm->GetValueDouble();
					break;
					case OSMTAG_TAG:	/* handle tags, k= v= */
					{
						unsigned int ti;
						unsigned int tnc;
						kGUIXMLItem *tparm;
						const char *k=0;
						const char *v=0;

						tnc=parm->GetNumChildren();
						for(ti=0;ti<tnc;++ti)
						{
							tparm=parm->GetChild(ti);
							itag=(int *)m_taghash.Find(tparm->GetName());
							if(itag)
							{
								switch(itag[0])
								{
								case OSMTAG_K:
									k=tparm->GetValueString();
								break;
								case OSMTAG_V:
									v=tparm->GetValueString();
								break;
								}
							}
						}
						/* lookup tag if we got both k & v*/
						if(k && v)
						{
							/* is this type in the ignore list?? */
							itag=(int *)m_taggroupspecialhash.Find(k);
							if(itag)
							{
								/* special handling */
								switch(itag[0])
								{
								case OSMTAG_PLACE_NAME:
								{
									int l;

									l=(int)strlen(v)+1;
									/* save name */
									node.m_name=(char *)m_heap.Alloc(l);
									memcpy(node.m_name,v,l);
								}
								break;
								case OSMTAG_STOPSIGN:
									/* special case look for n/s/e/w etc */
								break;
								case OSMTAG_YIELDSIGN:
									/* special case look for n/s/e/w etc */
								break;
								case OSMTAG_NOLEFTTURN:
									/* special case look for n/s/e/w etc */
								break;
								case OSMTAG_NORIGHTTURN:
									/* special case look for n/s/e/w etc */
								break;
								}
							}
							else
							{
								/* ok, lookup an enum for this tag */
								m_pair.Sprintf("%s^%s",k,v);
								ptag=(OSMTAG_DEF **)m_tagpairhash.Find(m_pair.GetString());
								if(ptag)
								{
									/* found tag/hash pair */
									if(numtags<=MAXTAGS)
										tags[numtags++]=ptag[0];
								}
								else
									AddUnknownPair(m_pair.GetString());
							}
						}
					}
					break;
					default:
						AddUnknownPair(parm->GetName());
					break;
					}
				}
				else
					AddUnknownPair(parm->GetName());
			}
			if(id)
			{
				OSMCONVNODE_DEF *np;

				/* lookup render object */
				node.m_renderindex=GetRenderIndex(numtags,tags);

				np=(OSMCONVNODE_DEF *)m_heap.Alloc(sizeof(OSMCONVNODE_DEF));
				memcpy(np,&node,sizeof(OSMCONVNODE_DEF));
				m_rs->m_nodeptrs.SetEntry(m_rs->m_numnodes++,np);				
				m_nodehash.Add(id,&np);
				
				if(++m_printcount==PRINTCOUNT)
				{
					kGUIString *s;

					m_printcount=0;
					s=new kGUIString();
					s->Sprintf("NumNodes=%d, NumWays=%d\n",m_rs->m_numnodes,m_rs->m_numways);
					while(m_comm.Write(&s)==false);
				}
			}
			parent->DelChild(child,false);
			PoolAdd(child);
		}
		break;
		case OSMTAG_WAY:
		{
			unsigned int i;
			unsigned int nc;
			kGUIXMLItem *parm;
			OSMCONVWAY_DEF way;
			OSMCONVWAY_DEF *wp;
			unsigned int numtags;
			OSMTAG_DEF *tags[MAXTAGS];
			OSMTAG_DEF **ptag;

			/* collect attributes for this node */
			way.m_numnodes=0;
			way.m_name=0;
			numtags=0;
			nc=child->GetNumChildren();
			for(i=0;i<nc;++i)
			{
				parm=child->GetChild(i);
				itag=(int *)m_taghash.Find(parm->GetName());
				if(itag)
				{
					switch(itag[0])
					{
					case OSMTAG_ID:	
					case OSMTAG_TIMESTAMP:	
					case OSMTAG_USER:
					case OSMTAG_VISIBLE:
						/* ignore these */
					break;
					case OSMTAG_ND:	/* ref= */
					{
						kGUIXMLItem *ref;
						OSMCONVNODE_DEF **pnp;

						/* we will assume that the first np parm if ref= */
						ref=parm->GetChild(0);
						pnp=(OSMCONVNODE_DEF **)m_nodehash.Find(ref->GetValueString());
						assert(pnp!=0,"Unknown node??\n");

						/* save to temp array until we know how many we need to allocate */
						m_tempnodeptrs.SetEntry(way.m_numnodes++,pnp[0]);
					}
					break;
					case OSMTAG_TAG:	/* handle tags, k= v= */
					{
						unsigned int ti;
						unsigned int tnc;
						kGUIXMLItem *tparm;
						const char *k=0;
						const char *v=0;

						tnc=parm->GetNumChildren();
						for(ti=0;ti<tnc;++ti)
						{
							tparm=parm->GetChild(ti);
							itag=(int *)m_taghash.Find(tparm->GetName());
							if(itag)
							{
								switch(itag[0])
								{
								case OSMTAG_K:
									k=tparm->GetValueString();
								break;
								case OSMTAG_V:
									v=tparm->GetValueString();
								break;
								}
							}
						}
						/* lookup tag if we got both k & v*/
						if(k && v)
						{
							/* is this type in the ignore list?? */
							itag=(int *)m_taggroupspecialhash.Find(k);
							if(itag)
							{
								/* special handling */
								switch(itag[0])
								{
								case OSMTAG_PLACE_NAME:
								{
									int l;

									l=(int)strlen(v)+1;
									/* save name */
									way.m_name=(char *)m_heap.Alloc(l);
									memcpy(way.m_name,v,l);
								}
								break;
								}
							}
							else
							{
								/* ok, lookup an enum for this tag */
								m_pair.Sprintf("%s^%s",k,v);
								ptag=(OSMTAG_DEF **)m_tagpairhash.Find(m_pair.GetString());
								if(ptag)
								{
									/* found tag/hash pair */
									if(numtags<MAXTAGS)
										tags[numtags++]=ptag[0];
								}
								else
									AddUnknownPair(m_pair.GetString());
							}
						}
					}
					break;
					default:
						AddUnknownPair(parm->GetName());
					break;
					}
				}
				else
					AddUnknownPair(parm->GetName());
			}

			/* allocate nodepointers in heap and point to them */
			if(way.m_numnodes)
			{
				unsigned int size;

				/* lookup render object */
				way.m_renderindex=GetRenderIndex(numtags,tags);

				/* if last point=first, then remove it and set the closed flag */
				if(m_tempnodeptrs.GetEntry(0)==m_tempnodeptrs.GetEntry(way.m_numnodes-1) && way.m_numnodes>1)
				{
					if(osmrenderinfo[way.m_renderindex].rendertype==OSMRENDERTYPE_POLYLINE)
					{
						OSMCONVNODE_DEF *anp;

						/* if a closed polyline then replace the last point so the curved */
						/* polylines can have a straight edge for the non rounded ends */
						anp=(OSMCONVNODE_DEF *)m_heap.Alloc(sizeof(OSMCONVNODE_DEF));
						anp->m_lat=(m_tempnodeptrs.GetEntry(0)->m_lat+m_tempnodeptrs.GetEntry(way.m_numnodes-2)->m_lat)/2.0f;
						anp->m_lon=(m_tempnodeptrs.GetEntry(0)->m_lon+m_tempnodeptrs.GetEntry(way.m_numnodes-2)->m_lon)/2.0f;
						anp->m_name=0;
						anp->m_renderindex=UNDEFINEDRENDERTYPE;
						anp->m_export=false;

						m_tempnodeptrs.InsertEntry(way.m_numnodes,0,1);
						m_tempnodeptrs.SetEntry(0,anp);
						m_tempnodeptrs.SetEntry(way.m_numnodes++,anp);
					}
					else
						--way.m_numnodes;
					way.m_closed=true;
				}
				else
					way.m_closed=false;

				if(way.m_numnodes)
				{
					size=way.m_numnodes*sizeof(OSMCONVNODE_DEF *);
					way.m_nodes=(OSMCONVNODE_DEF **)m_heap.Alloc(size);
					memcpy(way.m_nodes,m_tempnodeptrs.GetArrayPtr(),size);

					if(osmrenderinfo[way.m_renderindex].rendertype!=OSMRENDERTYPE_HIDE)
					{
						//debugging, add types to the name for this!
						if(way.m_renderindex==UNDEFINEDRENDERTYPE && numtags)
						{
							unsigned int tt;
							unsigned int l;

							m_pair.Clear();
							for(tt=0;tt<numtags;++tt)
							{
								if(tt)
									m_pair.Append(", ");
								m_pair.Append(tags[tt]->tag);
							}
							AddUnknownRender(m_pair.GetString());
							if(way.m_name)
							{
								m_pair.Append(", ");
								m_pair.Append(way.m_name);
							}
							l=m_pair.GetLen()+1;
							/* save name */
							way.m_name=(char *)m_heap.Alloc(l);
							memcpy(way.m_name,m_pair.GetString(),l);
						}
						/* copy way to the heap and save a pointer to it */
						wp=(OSMCONVWAY_DEF *)m_heap.Alloc(sizeof(OSMCONVWAY_DEF));
						memcpy(wp,&way,sizeof(OSMCONVWAY_DEF));
						m_rs->m_wayptrs.SetEntry(m_rs->m_numways++,wp);				
			
						if(++m_printcount==PRINTCOUNT)
						{
							kGUIString *s;

							m_printcount=0;
							s=new kGUIString();
							s->Sprintf("NumNodes=%d, NumWays=%d\n",m_rs->m_numnodes,m_rs->m_numways);
							while(m_comm.Write(&s)==false);
						}
					}
				}
			}
			parent->DelChild(child,false);
			PoolAdd(child);
		}
		break;
		}
	}
}

/******************************************************/
/* convert array of known tag pairs to a render index */
/******************************************************/

void OSMConvert::InitRenderLookup(void)
{
	unsigned int i;

	for(i=0;i<OSM_NUMTAGS;++i)
		m_hastag[i]=false;
}

unsigned int OSMConvert::GetRenderIndex(unsigned int numtags,OSMTAG_DEF **tags)
{
	unsigned int i;
	unsigned int j;
	const OSMRENDER_INFO *rp;
	unsigned int nummatches,bestmatches,bestindex;

	/* set tags to true */
	for(i=0;i<numtags;++i)
		m_hastag[tags[i]->id]=true;

	bestmatches=0;
	bestindex=UNDEFINEDRENDERTYPE;	/* last entry is default for unknown objects */
	rp=osmrenderinfo;
	for(i=0;i<NUMRENDERTYPES;++i)
	{
		nummatches=0;
		for(j=0;j<rp->numtags;++j)
		{
			if(m_hastag[rp->tags[j]])
				++nummatches;
		}
		if(nummatches==rp->numtags)
		{
			if(nummatches>bestmatches)
			{
				bestmatches=nummatches;
				bestindex=i;
			}
		}
		++rp;
	}

	/* set tags to false */
	for(i=0;i<numtags;++i)
		m_hastag[tags[i]->id]=false;

	return(bestindex);
}