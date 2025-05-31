#if defined _customentdata_included
    #endinput
#endif
#define _customentdata_included


#pragma reqlib customentdata
#if !defined AMXMODX_NOAUTOLOAD
    #pragma loadlib customentdata
#endif


/**
 * Sets custom entity data as cell
 *
 * @param entity            Entity to set data for
 * @param key               Identifier for data
 * @param data              Data to be set
 *
 * @noreturn
 */
native CED_SetCell(entity, const key[], any:data);

/**
 * Sets custom entity data as array
 *
 * @param entity            Entity to set data for
 * @param key               Identifier for data
 * @param data              Data to be set
 * @param size              Size of the data array
 *
 * @noreturn
 */
native CED_SetArray(entity, const key[], const any:data[], size);

/**
 * Sets custom entity data as string
 *
 * @param entity            Entity to set data for
 * @param key               Identifier for data
 * @param buffer            Data to be set
 *
 * @noreturn
 */
native CED_SetString(entity, const key[], const buffer[]);

/**
 * Gets custom entity data as cell
 *
 * @param entity            Entity to get data for
 * @param key               Identifier for data
 * @param data              Variable to put data in
 *
 * @return                  true if data identified by |key| exists, false otherwise
 */
native bool:CED_GetCell(entity, const key[], &any:data);

/**
 * Gets custom entity data as array
 *
 * @param entity            Entity to get data for
 * @param key               Identifier for data
 * @param data              Array to fill with data
 * @param size              Maximum size of |data| array
 *
 * @return                  true if data identified by |key| exists, false otherwise
 */
native bool:CED_GetArray(entity, const key[], any:data[], size);

/**
 * Gets custom entity data as string
 *
 * @param entity            Entity to get data for
 * @param key               Identifier for data
 * @param buffer            Buffer to put string data in
 * @param maxLength         Maximum length of |buffer| buffer
 *
 * @return                  true if data identified by |key| existed, false otherwise
 */
native bool:CED_GetString(entity, const key[], buffer[], maxLength);