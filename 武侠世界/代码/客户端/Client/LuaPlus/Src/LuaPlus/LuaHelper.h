///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2004 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://wwhiz.com/LuaPlus/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma once
#endif _MSC_VER
#ifndef LUAHELPER_H
#define LUAHELPER_H

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

/**
**/
namespace LuaHelper
{
	/**
		Attempts retrieval of the value from the passed in LuaStackObject.

		\param obj The LuaStackObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaStackObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\param defaultValue The default value to be returned if require is
			not true and the key doesn't exist or the found value is not
			of the right type.
		\return Returns the value found or the defaultValue.
	**/
	template< typename KeyT >
	static bool GetBoolean( LuaStackObject& obj, KeyT key, bool require = true, bool defaultValue = false )
	{
		LuaAutoObject boolObj = obj[ key ];
		if ( !boolObj.IsBoolean() )
		{
			if ( require )
				luaplus_assert( 0 );
			return defaultValue;
		}
		return boolObj.GetBoolean();
	}


	/**
		Attempts retrieval of the value from the passed in LuaStackObject.

		\param obj The LuaStackObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaStackObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\param defaultValue The default value to be returned if require is
			not true and the key doesn't exist or the found value is not
			of the right type.
		\return Returns the value found or the defaultValue.
	**/
	template< typename KeyT >
	static int GetInteger( LuaStackObject& obj, KeyT key, bool require = true, int defaultValue = -1 )
	{
		LuaAutoObject intObj = obj[ key ];
		if ( !intObj.IsInteger() )
		{
			if ( require )
				luaplus_assert( 0 );
			return defaultValue;
		}
		return intObj.GetInteger();
	}


	/**
		Attempts retrieval of the value from the passed in LuaStackObject.

		\param obj The LuaStackObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaStackObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\param defaultValue The default value to be returned if require is
			not true and the key doesn't exist or the found value is not
			of the right type.
		\return Returns the value found or the defaultValue.
	**/
	template< typename KeyT >
	static float GetFloat( LuaStackObject& obj, KeyT key, bool require = true, float defaultValue = -1.0f )
	{
		LuaAutoObject floatObj = obj[ key ];
		if ( !floatObj.IsNumber() )
		{
			if ( require )
				luaplus_assert( 0 );
			return defaultValue;
		}
		return floatObj.GetNumber();
	}


	/**
		Attempts retrieval of the value from the passed in LuaStackObject.

		\param obj The LuaStackObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaStackObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\param defaultValue The default value to be returned if require is
			not true and the key doesn't exist or the found value is not
			of the right type.
		\return Returns the value found or the defaultValue.
	**/
	template< typename KeyT >
	static void* GetLightUserData( LuaStackObject& obj, KeyT key, bool require = true, void* defaultValue = NULL )
	{
		LuaAutoObject outObj = obj[ key ];
		if ( !outObj.IsLightUserData() )
		{
			if ( require )
				luaplus_assert( 0 );
			return defaultValue;
		}
		return outObj.GetLightUserData();
	}


	/**
		Attempts retrieval of the value from the passed in LuaStackObject.

		\param obj The LuaStackObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaStackObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\param defaultValue The default value to be returned if require is
			not true and the key doesn't exist or the found value is not
			of the right type.
		\return Returns the value found or the defaultValue.
	**/
	template< typename KeyT >
	static const char* GetString( LuaStackObject& obj, KeyT key, bool require = true, const char* defaultValue = "" )
	{
		LuaAutoObject stringObj = obj[ key ];
		if ( !stringObj.IsString() )
		{
			if ( require )
				luaplus_assert( 0 );
			return defaultValue;
		}
		return stringObj.GetString();
	}


	/**
		Attempts retrieval of the obj from the passed in LuaStackObject.

		\param obj The LuaStackObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaStackObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\return Returns the object found.
	**/
	template< typename KeyT >
	static LuaStackObject GetTable( LuaStackObject& obj, KeyT key, bool require = true )
	{
		LuaStackObject tableObj = obj[ key ];
		if ( !tableObj.IsTable() )
		{
			if ( require )
				luaplus_assert( 0 );
		}
		return tableObj;
	}

	/**
		Attempts retrieval of the value from the passed in LuaPlus::LuaObject.

		\param obj The LuaPlus::LuaObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaPlus::LuaObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\param defaultValue The default value to be returned if require is
			not true and the key doesn't exist or the found value is not
			of the right type.
		\return Returns the value found or the defaultValue.
	**/
	template< typename KeyT >
	static bool GetBoolean( LuaPlus::LuaObject& obj, KeyT key, bool require = true, bool defaultValue = false )
	{
		LuaPlus::LuaObject boolObj = obj[ key ];
		if ( !boolObj.IsBoolean() )
		{
			if ( require )
				luaplus_assert( 0 );
			return defaultValue;
		}
		return boolObj.GetBoolean();
	}


	/**
		Attempts retrieval of the value from the passed in LuaPlus::LuaObject.

		\param obj The LuaPlus::LuaObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaPlus::LuaObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\param defaultValue The default value to be returned if require is
			not true and the key doesn't exist or the found value is not
			of the right type.
		\return Returns the value found or the defaultValue.
	**/
	template< typename KeyT >
	static int GetInteger( LuaPlus::LuaObject& obj, KeyT key, bool require = true, int defaultValue = -1 )
	{
		LuaPlus::LuaObject intObj = obj[ key ];
		if ( !intObj.IsInteger() )
		{
			if ( require )
				luaplus_assert( 0 );
			return defaultValue;
		}
		return intObj.GetInteger();
	}


	/**
		Attempts retrieval of the value from the passed in LuaPlus::LuaObject.

		\param obj The LuaPlus::LuaObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaPlus::LuaObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\param defaultValue The default value to be returned if require is
			not true and the key doesn't exist or the found value is not
			of the right type.
		\return Returns the value found or the defaultValue.
	**/
	template< typename KeyT >
	static float GetFloat( LuaPlus::LuaObject& obj, KeyT key, bool require = true, float defaultValue = -1.0f )
	{
		LuaPlus::LuaObject floatObj = obj[ key ];
		if ( !floatObj.IsNumber() )
		{
			if ( require )
				luaplus_assert( 0 );
			return defaultValue;
		}
		return floatObj.GetNumber();
	}


	/**
		Attempts retrieval of the value from the passed in LuaPlus::LuaObject.

		\param obj The LuaPlus::LuaObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaPlus::LuaObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\param defaultValue The default value to be returned if require is
			not true and the key doesn't exist or the found value is not
			of the right type.
		\return Returns the value found or the defaultValue.
	**/
	template< typename KeyT >
	static void* GetLightUserData( LuaPlus::LuaObject& obj, KeyT key, bool require = true, void* defaultValue = NULL )
	{
		LuaPlus::LuaObject outObj = obj[ key ];
		if ( !outObj.IsLightUserData() )
		{
			if ( require )
				luaplus_assert( 0 );
			return defaultValue;
		}
		return outObj.GetLightUserData();
	}


	/**
		Attempts retrieval of the value from the passed in LuaPlus::LuaObject.

		\param obj The LuaPlus::LuaObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaPlus::LuaObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\param defaultValue The default value to be returned if require is
			not true and the key doesn't exist or the found value is not
			of the right type.
		\return Returns the value found or the defaultValue.
	**/
	template< typename KeyT >
	static const char* GetString( LuaPlus::LuaObject& obj, KeyT key, bool require = true, const char* defaultValue = "" )
	{
		LuaPlus::LuaObject stringObj = obj[ key ];
		if ( !stringObj.IsString() )
		{
			if ( require )
				luaplus_assert( 0 );
			return defaultValue;
		}
		return stringObj.GetString();
	}


	/**
		Attempts retrieval of the obj from the passed in LuaPlus::LuaObject.

		\param obj The LuaPlus::LuaObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaPlus::LuaObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\return Returns the object found.
	**/
	template< typename KeyT >
	static LuaPlus::LuaObject GetTable( LuaPlus::LuaObject& obj, KeyT key, bool require = true )
	{
		LuaPlus::LuaObject tableObj = obj[ key ];
		if ( !tableObj.IsTable() )
		{
			if ( require )
				luaplus_assert( 0 );
		}
		return tableObj;
	}
};

} // namespace LuaPlus


#endif LUAHELPER_H
