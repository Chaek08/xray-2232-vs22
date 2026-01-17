////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_simulator_script.cpp
//	Created 	: 25.12.2002
//  Modified 	: 13.05.2004
//	Author		: Dmitriy Iassenev
//	Description : ALife Simulator script export
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "alife_simulator.h"
#include "script_space.h"
#include "ai_space.h"
#include "alife_object_registry.h"
#include "alife_story_registry.h"
#include "script_engine.h"

using namespace luabind;

typedef xr_vector<std::pair<shared_str,int> >	STORY_PAIRS;
extern	LPCSTR GAME_CONFIG;
LPCSTR	_INVALID_STORY_ID = "INVALID_STORY_ID";
STORY_PAIRS										story_ids;

CALifeSimulator *alife			()
{
	return			(const_cast<CALifeSimulator*>(ai().get_alife()));
}

CSE_ALifeDynamicObject *alife_object		(const CALifeSimulator *self, ALife::_OBJECT_ID id)
{
	VERIFY			(self);
	return			(self->objects().object(id,true));
}

CSE_ALifeDynamicObject *alife_object		(const CALifeSimulator *self, ALife::_OBJECT_ID id, bool no_assert)
{
	VERIFY			(self);
	return			(self->objects().object(id,no_assert));
}

CSE_ALifeDynamicObject *alife_story_object	(const CALifeSimulator *self, ALife::_STORY_ID id)
{
	return			(self->story_objects().object(id,true));
}

template <typename _id_type>
void generate_story_ids		(
	STORY_PAIRS &result,
	_id_type	INVALID_ID,
	LPCSTR		section_name,
	LPCSTR		INVALID_ID_STRING,
	LPCSTR		invalid_id_description,
	LPCSTR		invalid_id_redefinition,
	LPCSTR		duplicated_id_description
)
{
	result.clear			();

    CInifile				*Ini = 0;
    string_path				game_ltx;
    FS.update_path			(game_ltx,"$game_config$",GAME_CONFIG);
    R_ASSERT3				(FS.exist(game_ltx),"Couldn't find file",game_ltx);
    Ini						= xr_new<CInifile>(game_ltx);
    
    LPCSTR					N,V;
	u32 					k;
	shared_str				temp;
    LPCSTR					section = section_name;
    R_ASSERT				(Ini->section_exist(section));

	for (k = 0; Ini->r_line(section,k,&N,&V); ++k) {
		temp				= Ini->r_string_wb(section,N);
		
		R_ASSERT3			(!strchr(*temp,' '),invalid_id_description,*temp);
		R_ASSERT2			(xr_strcmp(*temp,INVALID_ID_STRING),invalid_id_redefinition);
		
		STORY_PAIRS::const_iterator	I = result.begin();
		STORY_PAIRS::const_iterator	E = result.end();
		for ( ; I != E; ++I)
			R_ASSERT3		((*I).first != temp,duplicated_id_description,*temp);
		
		result.push_back	(std::make_pair(*temp,atoi(N)));
	}

	result.push_back		(std::make_pair(INVALID_ID_STRING,INVALID_ID));

    xr_delete				(Ini);
}

void CALifeSimulator::script_register(lua_State *L)
{
	module(L)
	[
		class_<CALifeSimulator>("alife_simulator")
			.def("object",					(CSE_ALifeDynamicObject *(*) (const CALifeSimulator *,ALife::_OBJECT_ID))(alife_object))
			.def("object",					(CSE_ALifeDynamicObject *(*) (const CALifeSimulator *,ALife::_OBJECT_ID, bool))(alife_object))
			.def("story_object",			(CSE_ALifeDynamicObject *(*) (const CALifeSimulator *,ALife::_STORY_ID))(alife_story_object))
			.def("set_switch_online",		(void (CALifeSimulator::*) (ALife::_OBJECT_ID,bool))(&CALifeSimulator::set_switch_online))
			.def("set_switch_offline",		(void (CALifeSimulator::*) (ALife::_OBJECT_ID,bool))(&CALifeSimulator::set_switch_offline))
			.def("set_interactive",			(void (CALifeSimulator::*) (ALife::_OBJECT_ID,bool))(&CALifeSimulator::set_interactive)),

		def("alife",						&alife)
	];

	VERIFY					(story_ids.empty());
	generate_story_ids		(
		story_ids,
		INVALID_STORY_ID,
		"story_ids",
		"INVALID_STORY_ID",
		"Invalid story id description (contains spaces)!",
		"INVALID_STORY_ID redifinition!",
		"Duplicated story id description!"
	);

	luabind::class_<class_exporter<CALifeSimulator> >	instance("story_ids");

	STORY_PAIRS::const_iterator	I = story_ids.begin();
	STORY_PAIRS::const_iterator	E = story_ids.end();
	for ( ; I != E; ++I)
		instance.enum_			("_story_ids")[luabind::value(*(*I).first,(*I).second)];

	luabind::module				(L)[instance];
}

