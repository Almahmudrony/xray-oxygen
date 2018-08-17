////////////////////////////////////////////////////////////////////////////
//	Module 		: inventory_item.cpp
//	Created 	: 24.03.2003
//  Modified 	: 29.01.2004
//	Author		: Victor Reutsky, Yuri Dobronravin
//	Description : Inventory item
////////////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "stdafx.h"
#include "inventory_item.h"
#include "inventory_item_impl.h"
#include "inventory.h"
#include "physicsshellholder.h"
#include "entity_alive.h"
#include "Level.h"
#include "game_cl_base.h"
#include "Actor.h"
#include "string_table.h"
#include "../Include/xrRender/Kinematics.h"
#include "ai_object_location.h"
#include "object_broker.h"
#include "../xrEngine/igame_persistent.h"


#ifdef DEBUG
#	include "debug_renderer.h"
#endif

#define ITEM_REMOVE_TIME		30000

CInventoryItem::CInventoryItem() 
{
	m_flags.set			(Fbelt,false);
	m_flags.set			(Fruck,true);
	m_flags.set			(FRuckDefault,true);
	m_pInventory		= nullptr;

	SetDropManual		(FALSE);

	m_flags.set			(FCanTake,true);
	m_can_trade			= TRUE;
	m_flags.set			(FCanTrade, m_can_trade);
	m_flags.set			(FUsingCondition,false);
	m_fCondition		= 1.0f;

	m_name = m_nameShort = nullptr;

	m_ItemCurrPlace.value			= 0;
	m_ItemCurrPlace.type			= eItemPlaceUndefined;
	m_ItemCurrPlace.base_slot_id	= NO_ACTIVE_SLOT;
	m_ItemCurrPlace.slot_id			= NO_ACTIVE_SLOT;

	m_Description					= "";
	m_section_id					= nullptr;
	m_flags.set						(FIsHelperItem,false);
}

CInventoryItem::~CInventoryItem() 
{
#ifndef MASTER_GOLD
	bool B_GOOD			= (	!m_pInventory || 
							(std::find(	m_pInventory->m_all.begin(),m_pInventory->m_all.end(), this)==m_pInventory->m_all.end()) );
	if(!B_GOOD)
	{
		CObject* p	= object().H_Parent();
		Msg("inventory ptr is [%s]",m_pInventory?"not-null":"null");
		if(p)
			Msg("parent name is [%s]",p->cName().c_str());

			Msg("! ERROR item_id[%d] H_Parent=[%s][%d] [%d]",
				object().ID(),
				p ? p->cName().c_str() : "none",
				p ? p->ID() : -1,
				Device.dwFrame);
	}
#endif // #ifndef MASTER_GOLD
}

void CInventoryItem::Load(LPCSTR section) 
{
	CHitImmunity::LoadImmunities	(pSettings->r_string(section,"immunities_sect"),pSettings);

	ISpatial*			self				=	smart_cast<ISpatial*> (this);
	if (self)			self->spatial.type	|=	STYPE_VISIBLEFORAI;	

	m_section_id._set	( section );
	m_name				= CStringTable().translate( pSettings->r_string(section, "inv_name") );
	m_nameShort			= CStringTable().translate( pSettings->r_string(section, "inv_name_short") );

	m_weight			= pSettings->r_float(section, "inv_weight");
	R_ASSERT			(m_weight>=0.f);

	m_cost				= pSettings->r_u32(section, "cost");
	u32 sl  			= pSettings->r_u32(section,"slot");
	m_ItemCurrPlace.base_slot_id = (sl==-1)?0:(sl+1);

	m_Description = CStringTable().translate( pSettings->r_string(section, "description") );

	m_flags.set(Fbelt,			READ_IF_EXISTS(pSettings, r_bool, section, "belt",		FALSE));
	m_can_trade = READ_IF_EXISTS(pSettings, r_bool, section, "can_take",	TRUE);
	m_flags.set(FCanTake,		m_can_trade);
	m_flags.set(FCanTrade,		READ_IF_EXISTS(pSettings, r_bool, section, "can_trade",	TRUE));
	m_flags.set(FIsQuestItem,	READ_IF_EXISTS(pSettings, r_bool, section, "quest_item",FALSE));
    m_highlight_equipped = !!READ_IF_EXISTS(pSettings, r_bool, section, "highlight_equipped", FALSE);
	
	if ( BaseSlot() != NO_ACTIVE_SLOT || Belt())
	{
		m_flags.set					(FRuckDefault, pSettings->r_bool(section, "default_to_ruck" ));
		m_flags.set					(FAllowSprint, pSettings->r_bool(section, "sprint_allowed" ));
		m_fControlInertionFactor	= pSettings->r_float(section,"control_inertion_factor");
	}
	m_icon_name					= READ_IF_EXISTS(pSettings, r_string,section,"icon_name",		NULL);

}

void  CInventoryItem::ChangeCondition(float fDeltaCondition)
{
	m_fCondition += fDeltaCondition;
	clamp(m_fCondition, 0.f, 1.f);
}


void	CInventoryItem::Hit					(SHit* pHDS)
{
	if( !m_flags.test(FUsingCondition) ) return;

	float hit_power = pHDS->damage();
	hit_power *= GetHitImmunity(pHDS->hit_type);

	ChangeCondition(-hit_power);
}

LPCSTR CInventoryItem::NameItem() 
{
	return m_name.c_str();
}

LPCSTR CInventoryItem::NameShort() 
{
	return m_nameShort.c_str();
}

bool CInventoryItem::Useful() const
{
	return CanTake();
}

bool CInventoryItem::ActivateItem() 
{
	return false;
}

void CInventoryItem::DeactivateItem() 
{
}

void CInventoryItem::OnH_B_Independent(bool just_before_destroy)
{
	m_ItemCurrPlace.type = eItemPlaceUndefined ;
	UpdateXForm();
}

void CInventoryItem::OnH_A_Independent()
{
	m_dwItemIndependencyTime	= Level().timeServer();
	m_ItemCurrPlace.type		= eItemPlaceUndefined;	
	inherited::OnH_A_Independent();
}

void CInventoryItem::OnH_B_Chield()
{
}

void CInventoryItem::OnH_A_Chield()
{
	inherited::OnH_A_Chield		();
}
#ifdef DEBUG
extern	Flags32	dbg_net_Draw_Flags;
#endif

void CInventoryItem::UpdateCL()
{
#ifdef DEBUG
	if(bDebug){
		if (dbg_net_Draw_Flags.test(dbg_draw_invitem) )
		{
			Device.seqRender.Remove(this);
			Device.seqRender.Add(this);
		}else
			Device.seqRender.Remove(this);
	}

#endif
}

void CInventoryItem::OnEvent (NET_Packet& P, u16 type)
{
	switch (type)
	{
	case GE_ADDON_ATTACH:
		{
			u16 ItemID;
			P.r_u16			(ItemID);
			CInventoryItem*	 ItemToAttach	= smart_cast<CInventoryItem*>(Level().Objects.net_Find(ItemID));
			if (!ItemToAttach) break;
			Attach(ItemToAttach,true);
		}break;
	case GE_ADDON_DETACH:
		{
			string64			i_name;
			P.r_stringZ			(i_name);
			Detach(i_name, true);
		}break;	
	case GE_CHANGE_POS:
		{
			Fvector p; 
			P.r_vec3(p);
			CPHSynchronize* pSyncObj = nullptr;
			pSyncObj = object().PHGetSyncItem(0);
			if (!pSyncObj) return;
			SPHNetState state;
			pSyncObj->get_State(state);
			state.position = p;
			state.previous_position = p;
			pSyncObj->set_State(state);

		}break;
	}
}

//������� ������������ ���� ����������� � ������ ����� ���� 
//� ��������� � ��������� ��������������� ������ � ������������
//�������, ������� ������� ������ ���� ��������������
bool CInventoryItem::Detach(const char* item_section_name, bool b_spawn_item) 
{
	if(b_spawn_item)
	{
		CSE_Abstract*		D	= F_entity_Create(item_section_name);
		R_ASSERT		   (D);
		CSE_ALifeDynamicObject	*l_tpALifeDynamicObject = 
								smart_cast<CSE_ALifeDynamicObject*>(D);
		R_ASSERT			(l_tpALifeDynamicObject);
		
		l_tpALifeDynamicObject->m_tNodeID = object().ai_location().level_vertex_id();
			
		// Fill
		D->s_name			=	item_section_name;
		D->set_name_replace	("");
		D->s_RP				=	0xff;
		D->ID				=	0xffff;
		
		D->ID_Parent		=	u16(object().H_Parent()->ID());
		
		D->ID_Phantom		=	0xffff;
		D->o_Position		=	object().Position();
		D->s_flags.assign	(M_SPAWN_OBJECT_LOCAL);
		D->RespawnTime		=	0;
		// Send
		NET_Packet			P;
		D->Spawn_Write		(P,TRUE);
		Level().Send		(P);
		// Destroy
		F_entity_Destroy	(D);
	}
	return true;
}

/////////// network ///////////////////////////////
BOOL CInventoryItem::net_Spawn			(CSE_Abstract* DC)
{
	VERIFY							(!m_pInventory);

	m_flags.set						(FInInterpolation, false);
	m_flags.set						(FInInterpolate,	false);

	m_flags.set						(Fuseful_for_NPC, true);
	CSE_Abstract					*e	= (CSE_Abstract*)(DC);
	CSE_ALifeObject					*alife_object = smart_cast<CSE_ALifeObject*>(e);
	if (alife_object)
		m_flags.set(Fuseful_for_NPC, alife_object->m_flags.test(CSE_ALifeObject::flUsefulForAI));

	CSE_ALifeInventoryItem			*pSE_InventoryItem = smart_cast<CSE_ALifeInventoryItem*>(e);
	if (!pSE_InventoryItem)			return TRUE;

	//!!!
	m_fCondition = pSE_InventoryItem->m_fCondition;
	net_Spawn_install_upgrades( pSE_InventoryItem->m_upgrades );

	m_dwItemIndependencyTime		= 0;
	
	m_just_after_spawn		= true;
	m_activated				= false;
	return							TRUE;
}

void CInventoryItem::net_Destroy		()
{
	if(m_pInventory)
		VERIFY(std::find(m_pInventory->m_all.begin(), m_pInventory->m_all.end(), this)==m_pInventory->m_all.end() );
}

void CInventoryItem::save(NET_Packet &packet)
{
	packet.w_u16			(m_ItemCurrPlace.value);
	packet.w_float			(m_fCondition);

	if (object().H_Parent()) 
	{
		packet.w_u8			(0);
		return;
	}

	u8 _num_items			= (u8)object().PHGetSyncItemsNumber(); 
	packet.w_u8				(_num_items);
	object().PHSaveState	(packet);
}

void CInventoryItem::net_Export(NET_Packet& P)
{
	//copy from CPhysicObject
	P.w_u8(0);
};

void CInventoryItem::load(IReader &packet)
{
	m_ItemCurrPlace.value	= packet.r_u16();
	m_fCondition			= packet.r_float();

	u8						tmp = packet.r_u8();
	if (!tmp)
		return;
	
	if (!object().PPhysicsShell()) 
	{
		object().setup_physic_shell	();
		object().PPhysicsShell()->Disable();
	}
	
	object().PHLoadState(packet);
	object().PPhysicsShell()->Disable();
}

void CInventoryItem::reload		(LPCSTR section)
{
	inherited::reload		(section);
	m_holder_range_modifier	= READ_IF_EXISTS(pSettings,r_float,section,"holder_range_modifier",1.f);
	m_holder_fov_modifier	= READ_IF_EXISTS(pSettings,r_float,section,"holder_fov_modifier",1.f);
}

void CInventoryItem::reinit		()
{
	m_pInventory	= nullptr;
	m_ItemCurrPlace.type = eItemPlaceUndefined;
}

bool CInventoryItem::can_kill			() const
{
	return				(false);
}

CInventoryItem *CInventoryItem::can_kill	(CInventory *inventory) const
{
	return				(nullptr);
}

const CInventoryItem *CInventoryItem::can_kill			(const xr_vector<const CGameObject*> &items) const
{
	return				(nullptr);
}

CInventoryItem *CInventoryItem::can_make_killing	(const CInventory *inventory) const
{
	return				(nullptr);
}

bool CInventoryItem::ready_to_kill		() const
{
	return				(false);
}

void CInventoryItem::activate_physic_shell()
{
	CEntityAlive*	E		= smart_cast<CEntityAlive*>(object().H_Parent());
	if (!E) {
		on_activate_physic_shell();
		return;
	};

	UpdateXForm();

	object().CPhysicsShellHolder::activate_physic_shell();
}

void CInventoryItem::UpdateXForm	()
{
	if (!object().H_Parent())	return;

	// Get access to entity and its visual
	CEntityAlive*	E		= smart_cast<CEntityAlive*>(object().H_Parent());
	if (!E) return;
	
	if (E->cast_base_monster()) return;

	const CInventoryOwner	*parent = smart_cast<const CInventoryOwner*>(E);
	if (parent && parent->use_simplified_visual())
		return;

	if (parent->attached(this))
		return;

	R_ASSERT		(E);
	IKinematics*	V		= smart_cast<IKinematics*>	(E->Visual());
	VERIFY			(V);

	// Get matrices
	int						boneL = -1, boneR = -1, boneR2 = -1;
	E->g_WeaponBones(boneL,boneR,boneR2);
	if (boneR == -1)	return;
#pragma todo("TO ALL: serious performance problem")
	V->CalculateBones	();
	Fmatrix& mL			= V->LL_GetTransform(u16(boneL));
	Fmatrix& mR			= V->LL_GetTransform(u16(boneR));
	// Calculate
	Fmatrix			mRes;
	Fvector			R,D,N;
	D.sub			(mL.c,mR.c);	D.normalize_safe();

	if(fis_zero(D.magnitude()))
	{
		mRes.set(E->XFORM());
		mRes.c.set(mR.c);
	}
	else
	{		
		D.normalize();
		R.crossproduct	(mR.j,D);

		N.crossproduct	(D,R);
		N.normalize();

		mRes.set		(R,N,D,mR.c);
		mRes.mulA_43	(E->XFORM());
	}

	object().Position().set(mRes.c);
}

#ifdef DEBUG

void CInventoryItem::OnRender()
{
	if (bDebug && object().Visual())
	{
		if (!(dbg_net_Draw_Flags.is_any(dbg_draw_invitem))) return;

		Fvector bc,bd; 
		object().Visual()->getVisData().box.get_CD	(bc,bd);
		Fmatrix	M = object().XFORM();
		M.c.add (bc);
		Level().debug_renderer().draw_obb			(M,bd,color_rgba(0,0,255,255));
	};
}
#endif

DLL_Pure *CInventoryItem::_construct	()
{
	m_object	= smart_cast<CPhysicsShellHolder*>(this);
	VERIFY		(m_object);
	return		(inherited::_construct());
}

void CInventoryItem::modify_holder_params	(float &range, float &fov) const
{
	range		*= m_holder_range_modifier;
	fov			*= m_holder_fov_modifier;
}

ALife::_TIME_ID	 CInventoryItem::TimePassedAfterIndependant()	const
{
	return (!object().H_Parent() && m_dwItemIndependencyTime != 0) ? (Level().timeServer() - m_dwItemIndependencyTime) : 0;
}

bool	CInventoryItem::CanTrade() const 
{
	bool res = true;
#pragma todo("Dima to Andy : why CInventoryItem::CanTrade can be called for the item, which doesn't have owner?")
	if(m_pInventory)
		res = inventory_owner().AllowItemToTrade(this,m_ItemCurrPlace);

	return (res && m_flags.test(FCanTrade) && !IsQuestItem());
}

Frect CInventoryItem::GetKillMsgRect() const
{
	float x,y,w,h;

	x = READ_IF_EXISTS(pSettings,r_float,m_object->cNameSect(),"kill_msg_x", 0.0f);
	y = READ_IF_EXISTS(pSettings,r_float,m_object->cNameSect(),"kill_msg_y", 0.0f);
	w = READ_IF_EXISTS(pSettings,r_float,m_object->cNameSect(),"kill_msg_width", 0.0f);
	h = READ_IF_EXISTS(pSettings,r_float,m_object->cNameSect(),"kill_msg_height", 0.0f);

	return Frect().set(x,y,w,h);
}

Irect CInventoryItem::GetInvGridRect() const
{
	u32 x,y,w,h;

	x = pSettings->r_u32(m_object->cNameSect(),"inv_grid_x");
	y = pSettings->r_u32(m_object->cNameSect(),"inv_grid_y");
	w = pSettings->r_u32(m_object->cNameSect(),"inv_grid_width");
	h = pSettings->r_u32(m_object->cNameSect(),"inv_grid_height");

	return Irect().set(x,y,w,h);
}

Irect CInventoryItem::GetUpgrIconRect() const
{
	u32 x,y,w,h;

	x = READ_IF_EXISTS(pSettings,r_u32,m_object->cNameSect(),"upgr_icon_x", 0);
	y = READ_IF_EXISTS(pSettings,r_u32,m_object->cNameSect(),"upgr_icon_y", 0);
	w = READ_IF_EXISTS(pSettings,r_u32,m_object->cNameSect(),"upgr_icon_width", 0);
	h = READ_IF_EXISTS(pSettings,r_u32,m_object->cNameSect(),"upgr_icon_height", 0);

	return Irect().set(x,y,w,h);
}

bool CInventoryItem::IsNecessaryItem(CInventoryItem* item)		
{
	return IsNecessaryItem(item->object().cNameSect());
};

BOOL CInventoryItem::IsInvalid() const
{
	return object().getDestroy() || GetDropManual();
}

u16 CInventoryItem::object_id()const
{
	return object().ID();
}

u16 CInventoryItem::parent_id() const
{
	return (object().H_Parent()) ? object().H_Parent()->ID() : u16(-1);
}

void CInventoryItem::SetDropManual(BOOL val)
{
	m_flags.set(FdropManual, val);
}

bool CInventoryItem::has_network_synchronization	() const
{
	return false;
}