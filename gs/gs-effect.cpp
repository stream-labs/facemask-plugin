/*
 * Face Masks for SlOBS
 * Copyright (C) 2017 General Workings Inc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "gs-effect.h"

const size_t GS::Effect::MAX_POOL_SIZE = 10;
std::map<std::string, std::pair<size_t, gs_effect_t*> > GS::Effect::pool;


void GS::Effect::add_to_cache(std::string name, gs_effect_t *effect) {
	
	if (pool.find(name) != pool.end()) {
		throw "Incorrect use of add_to_cache, before calling load_from_cache";
	}

	if (pool.size() < MAX_POOL_SIZE)
	{
		//blog(LOG_DEBUG, "Caching effect: %s", name.c_str());
		pool[name] = std::make_pair(1, effect);
	}
	else
	{
		// remove the least re-used effect
		auto min_it = pool.begin();
		size_t min_ref = min_it->second.first;
		for (auto it = pool.begin(); it != pool.end(); it++)
		{
			if (min_ref > it->second.first)
			{
				min_ref = it->second.first;
				min_it = it;
			}
		}
		pool.erase(min_it);
		//blog(LOG_DEBUG, "Pool full. Removing least used effect: %s, #ref = %d", min_it->first.c_str(), min_it->second.first);

		pool[name] = std::make_pair(1, effect);
	}
}

void GS::Effect::load_from_cache(std::string name, gs_effect_t **effect_ptr) {
	if (pool.find(name) != pool.end()) {
		//blog(LOG_DEBUG, "Re-using effect: %s", name.c_str());
		pool[name].first++;
		*effect_ptr = pool[name].second;
	}
	else
		*effect_ptr = nullptr;
}

void GS::Effect::unload_effect(std::string name, gs_effect_t *effect) {
	// only destroy if the effect is not managed by the pool
	if (pool.find(name) == pool.end()) {
		//blog(LOG_DEBUG, "Destroying unmanaged effect: %s", name.c_str());
		obs_enter_graphics();
		gs_effect_destroy(effect);
		obs_leave_graphics();
		return;
	}

	//blog(LOG_DEBUG, "Delaying destroying managed effect: %s", name.c_str());

}

void GS::Effect::destroy_pool() {
	for (auto &ent : pool) {
		//blog(LOG_DEBUG, "POOL DESTROY: destroying managed effect: %s", ent.first.c_str());
		obs_enter_graphics();
		gs_effect_destroy(ent.second.second);
		obs_leave_graphics();
	}
}

GS::Effect::Effect(std::string file) {
	m_name = file;
	GS::Effect::load_from_cache(m_name, &m_effect);
	if (m_effect == nullptr)
	{
		obs_enter_graphics();
		char* errorMessage = nullptr;
		m_effect = gs_effect_create_from_file(m_name.c_str(), &errorMessage);
		if (!m_effect || errorMessage) {
			std::string error(errorMessage);
			bfree((void*)errorMessage);
			obs_leave_graphics();
			throw std::runtime_error(error);
		}
		obs_leave_graphics();

		GS::Effect::add_to_cache(m_name, m_effect);
	}
}

GS::Effect::Effect(std::string code, std::string name) {
	m_name = name;
	GS::Effect::load_from_cache(m_name, &m_effect);
	if (m_effect == nullptr)
	{
		obs_enter_graphics();
		char* errorMessage = nullptr;
		m_effect = gs_effect_create(code.c_str(), m_name.c_str(), &errorMessage);
		if (!m_effect || errorMessage) {
			std::string error(errorMessage);
			bfree((void*)errorMessage);
			obs_leave_graphics();
			throw std::runtime_error(error);
		}
		obs_leave_graphics();

		GS::Effect::add_to_cache(m_name, m_effect);
	}
}

GS::Effect::~Effect() {
	GS::Effect::unload_effect(m_name, m_effect);
}

gs_effect_t* GS::Effect::GetObject() {
	return m_effect;
}

std::vector<GS::EffectParameter> GS::Effect::GetParameters() {
	size_t num = gs_effect_get_num_params(m_effect);
	std::vector<GS::EffectParameter> ps;
	ps.reserve(num);
	for (size_t idx = 0; idx < num; idx++) {
		ps.emplace_back(EffectParameter(gs_effect_get_param_by_idx(m_effect, idx)));
	}
	return ps;
}

GS::EffectParameter GS::Effect::GetParameterByName(std::string name) {
	gs_eparam_t* param = gs_effect_get_param_by_name(m_effect, name.c_str());
	if (!param)
		throw std::invalid_argument("parameter with name not found");
	return EffectParameter(param);
}

GS::EffectParameter::EffectParameter(gs_eparam_t* param) {
	if (!param)
		throw std::invalid_argument("param is null");

	m_param = param;
	gs_effect_get_param_info(m_param, &m_paramInfo);
}

GS::EffectParameter::Type GS::EffectParameter::GetType() {
	switch (m_paramInfo.type) {
		case GS_SHADER_PARAM_BOOL:
			return Type::Boolean;
		case GS_SHADER_PARAM_FLOAT:
			return Type::Float;
		case GS_SHADER_PARAM_VEC2:
			return Type::Float2;
		case GS_SHADER_PARAM_VEC3:
			return Type::Float3;
		case GS_SHADER_PARAM_VEC4:
			return Type::Float4;
		case GS_SHADER_PARAM_INT:
			return Type::Integer;
		case GS_SHADER_PARAM_INT2:
			return Type::Integer2;
		case GS_SHADER_PARAM_INT3:
			return Type::Integer3;
		case GS_SHADER_PARAM_INT4:
			return Type::Integer4;
		case GS_SHADER_PARAM_MATRIX4X4:
			return Type::Matrix;
		case GS_SHADER_PARAM_TEXTURE:
			return Type::Texture;
		//case GS_SHADER_PARAM_STRING:
		//	return Type::String;
		default:
		case GS_SHADER_PARAM_UNKNOWN:
			return Type::Unknown;
	}
}

void GS::EffectParameter::SetBoolean(bool v) {
	if (GetType() != Type::Boolean)
		throw std::bad_cast();
	gs_effect_set_bool(m_param, v);
}

void GS::EffectParameter::SetBooleanArray(bool v[], size_t sz) {
	if (GetType() != Type::Boolean)
		throw std::bad_cast();
	gs_effect_set_val(m_param, v, sz);
}

void GS::EffectParameter::SetFloat(float_t x) {
	if (GetType() != Type::Float)
		throw std::bad_cast();
	gs_effect_set_float(m_param, x);
}

void GS::EffectParameter::SetFloat2(vec2& v) {
	if (GetType() != Type::Float2)
		throw std::bad_cast();
	gs_effect_set_vec2(m_param, &v);
}

void GS::EffectParameter::SetFloat2(float_t x, float_t y) {
	if (GetType() != Type::Float2)
		throw std::bad_cast();
	vec2 v = { x, y };
	gs_effect_set_vec2(m_param, &v);
}

void GS::EffectParameter::SetFloat3(vec3& v) {
	if (GetType() != Type::Float3)
		throw std::bad_cast();
	gs_effect_set_vec3(m_param, &v);
}

void GS::EffectParameter::SetFloat3(float_t x, float_t y, float_t z) {
	if (GetType() != Type::Float3)
		throw std::bad_cast();
	vec3 v = { x, y, z };
	gs_effect_set_vec3(m_param, &v);
}

void GS::EffectParameter::SetFloat4(vec4& v) {
	if (GetType() != Type::Float4)
		throw std::bad_cast();
	gs_effect_set_vec4(m_param, &v);
}

void GS::EffectParameter::SetFloat4(float_t x, float_t y, float_t z, float_t w) {
	if (GetType() != Type::Float4)
		throw std::bad_cast();
	vec4 v = { x, y, z, w };
	gs_effect_set_vec4(m_param, &v);
}

void GS::EffectParameter::SetFloatArray(float_t v[], size_t sz) {
	// this could be Float2/Float3/Float4
	//if (GetType() != Type::Float)
	//	throw std::bad_cast();
	gs_effect_set_val(m_param, v, sz);
}

void GS::EffectParameter::SetInteger(int32_t x) {
	if (GetType() != Type::Integer)
		throw std::bad_cast();
	gs_effect_set_int(m_param, x);
}

void GS::EffectParameter::SetInteger2(int32_t x, int32_t y) {
	if (GetType() != Type::Integer2)
		throw std::bad_cast();
	int32_t v[] = { x, y };
	gs_effect_set_val(m_param, v, 2);
}

void GS::EffectParameter::SetInteger3(int32_t x, int32_t y, int32_t z) {
	if (GetType() != Type::Integer3)
		throw std::bad_cast();
	int32_t v[] = { x, y, z };
	gs_effect_set_val(m_param, v, 3);
}

void GS::EffectParameter::SetInteger4(int32_t x, int32_t y, int32_t z, int32_t w) {
	if (GetType() != Type::Integer4)
		throw std::bad_cast();
	int32_t v[] = { x, y, z, w };
	gs_effect_set_val(m_param, v, 4);
}

void GS::EffectParameter::SetIntegerArray(int32_t v[], size_t sz) {
	if (GetType() != Type::Integer)
		throw std::bad_cast();
	gs_effect_set_val(m_param, v, sz);
}

void GS::EffectParameter::SetMatrix(matrix4& v) {
	if (GetType() != Type::Matrix)
		throw std::bad_cast();
	gs_effect_set_matrix4(m_param, &v);
}

void GS::EffectParameter::SetTexture(std::shared_ptr<GS::Texture> v) {
	if (GetType() != Type::Texture)
		throw std::bad_cast();
	gs_effect_set_texture(m_param, v->GetObject());
}

void  GS::EffectParameter::SetSampler(gs_sampler_state* ss) {
	if (GetType() != Type::Texture)
		throw std::bad_cast();
	gs_effect_set_next_sampler(m_param, ss);
}
