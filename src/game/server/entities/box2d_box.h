/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_BOX2D_BOX_H
#define GAME_SERVER_ENTITIES_BOX2D_BOX_H

#include <box2d/box2d.h>
#include <game/server/entity.h>

class CBox2DBox : public CEntity
{
public:
	CBox2DBox(CGameWorld *pGameWorld, vec2 Pos, vec2 Size, float Angle, b2World* World, b2BodyType bodytype, float dens);
	~CBox2DBox();

	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	b2Body* getBody() { return m_Body; }

private:
	b2World* m_World;
	b2Body* m_Body;
	vec2 m_Size;
	int m_ID2, m_ID3, m_ID4; // for the other box corners
};

#endif
