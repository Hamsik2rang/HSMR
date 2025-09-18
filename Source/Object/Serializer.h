//
//  Serializer.h
//  HSMR
//
//  Created by Yongsik Im on 9/11/25.
//
#ifndef __HS_SERIALIZER_H__
#define __HS_SERIALIZER_H__

#include "Precompile.h"

HS_NS_BEGIN

class HS_OBJECT_API Serializable
{
public:
	virtual bool Serialize(const std::string& path) const = 0;
	virtual bool Deserialize(const std::string& path) = 0;
};

class HS_OBJECT_API Serializer
{

};

HS_NS_END

#endif