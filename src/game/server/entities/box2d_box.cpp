/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#define SCALE 30

#include "box2d_box.h"
#include <math.h>

#include <game/generated/protocol.h>

#include <engine/shared/config.h>
#include <game/server/teams.h>


void Rotate(vec2* vertex, float x_orig, float y_orig, float angle)
{
	// FUCK THIS MATH
	float s = sin(angle);
	float c = cos(angle);

	vertex->x -= x_orig;
	vertex->y -= y_orig;

	float xnew = vertex->x * c - vertex->y * s;
	float ynew = vertex->x * s + vertex->y * c;

	vertex->x = xnew + x_orig;
	vertex->y = ynew + y_orig;
}


CBox2DBox::CBox2DBox(CGameWorld *pGameWorld, vec2 Pos, vec2 Size, float Angle, b2World* World, b2BodyType bodytype, float dens) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER)
{
	m_Pos = Pos;
	m_Size = Size;
	m_World = World;

	// the box
 	b2BodyDef BodyDef;
	BodyDef.position = b2Vec2(Pos.x / SCALE, Pos.y / SCALE);
	BodyDef.type = bodytype;
	m_Body = GameServer()->m_b2world->CreateBody(&BodyDef);

	b2PolygonShape Shape;
	Shape.SetAsBox(Size.x / 2 / SCALE, Size.y / 2 / SCALE);
	b2FixtureDef FixtureDef;
	FixtureDef.density = dens;
	FixtureDef.shape = &Shape;
	m_Body->CreateFixture(&FixtureDef);

	m_ID2 = Server()->SnapNewID();
	m_ID3 = Server()->SnapNewID();
	m_ID4 = Server()->SnapNewID();

	GameWorld()->InsertEntity(this);
}

CBox2DBox::~CBox2DBox()
{
	Server()->SnapFreeID(m_ID2);
	Server()->SnapFreeID(m_ID3);
	Server()->SnapFreeID(m_ID4);
	if (GameServer()->m_b2world)
	{
		m_World->DestroyBody(m_Body);
	}

	for (unsigned i=0; i<GameServer()->m_b2bodies.size(); i++)
	{
		if (GameServer()->m_b2bodies[i] == this)
		{
			GameServer()->m_b2bodies.erase(GameServer()->m_b2bodies.begin() + i);
			break;
		}
	}
}

void CBox2DBox::Tick()
{
	m_Pos = vec2(m_Body->GetPosition().x * SCALE, m_Body->GetPosition().y * SCALE);
	if (GameLayerClipped(m_Pos))
	{
		m_MarkedForDestroy = true;
	}
}

void CBox2DBox::TickPaused()
{

}

void CBox2DBox::Snap(int SnappingClient)
{
	vec2 pos(m_Body->GetPosition().x * SCALE, m_Body->GetPosition().y * SCALE);
	vec2 vertices[4] = {
		vec2(pos.x - (m_Size.x/2), pos.y - (m_Size.y/2)),
		vec2(pos.x + (m_Size.x/2), pos.y - (m_Size.y/2)),
		vec2(pos.x + (m_Size.x/2), pos.y + (m_Size.y/2)),
		vec2(pos.x - (m_Size.x/2), pos.y + (m_Size.y/2))
	};

	if (NetworkClipped(SnappingClient) and NetworkClipped(SnappingClient, vertices[0]) and NetworkClipped(SnappingClient, vertices[1]) and NetworkClipped(SnappingClient, vertices[2]) and NetworkClipped(SnappingClient, vertices[3]))
		return;

	CNetObj_Laser *pObj1 = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, GetID(), sizeof(CNetObj_Laser)));
	CNetObj_Laser *pObj2 = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_ID2, sizeof(CNetObj_Laser)));
	CNetObj_Laser *pObj3 = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_ID3, sizeof(CNetObj_Laser)));
	CNetObj_Laser *pObj4 = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_ID4, sizeof(CNetObj_Laser)));
	if(!pObj1 or !pObj2 or !pObj3 or !pObj4)
		return;

	float angle = m_Body->GetAngle(); // radians

	// FUCK THIS MATH
	for (int i=0; i<4; i++)
	{
		Rotate(&vertices[i], pos.x, pos.y, angle);
	}

	pObj1->m_X = (int)vertices[1].x;
	pObj1->m_Y = (int)vertices[1].y;
	pObj1->m_FromX = (int)vertices[0].x;
	pObj1->m_FromY = (int)vertices[0].y;
	pObj2->m_X = (int)vertices[2].x;
	pObj2->m_Y = (int)vertices[2].y;
	pObj2->m_FromX = (int)vertices[1].x;
	pObj2->m_FromY = (int)vertices[1].y;
	pObj3->m_X = (int)vertices[3].x;
	pObj3->m_Y = (int)vertices[3].y;
	pObj3->m_FromX = (int)vertices[2].x;
	pObj3->m_FromY = (int)vertices[2].y;
	pObj4->m_X = (int)vertices[0].x;
	pObj4->m_Y = (int)vertices[0].y;
	pObj4->m_FromX = (int)vertices[3].x;
	pObj4->m_FromY = (int)vertices[3].y;

	pObj1->m_StartTick = Server()->Tick();
	pObj2->m_StartTick = Server()->Tick();
	pObj3->m_StartTick = Server()->Tick();
	pObj4->m_StartTick = Server()->Tick();
}
