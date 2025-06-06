"""Binary Sensor component for Itho."""

from homeassistant.components import mqtt
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity_platform import AddEntitiesCallback

from .const import _LOGGER, CONF_ADDON_TYPE, CONF_ENTITIES_CREATION_MODE
from .sensors.autotemp import get_autotemp_binary_sensors
from .sensors.fan import get_cve_binary_sensors, get_noncve_binary_sensors
from .sensors.wpu import get_wpu_binary_sensors


async def async_setup_entry(
    hass: HomeAssistant,
    config_entry: ConfigEntry,
    async_add_entities: AddEntitiesCallback,
) -> None:
    """Set up Itho add-on binary sensors from config entry based on their type."""

    if not await mqtt.async_wait_for_mqtt_client(hass):
        _LOGGER.error("MQTT integration is not available")
        return

    sensors = []
    selected_sensors = []
    if config_entry.data[CONF_ADDON_TYPE] == "autotemp":
        sensors.extend(get_autotemp_binary_sensors(config_entry))

    if config_entry.data[CONF_ADDON_TYPE] == "cve":
        sensors.extend(get_cve_binary_sensors(config_entry))

    if config_entry.data[CONF_ADDON_TYPE] == "noncve":
        sensors.extend(get_noncve_binary_sensors(config_entry))

    if config_entry.data[CONF_ADDON_TYPE] == "wpu":
        sensors.extend(get_wpu_binary_sensors(config_entry))

    for sensor in sensors:
        if (
            config_entry.data[CONF_ENTITIES_CREATION_MODE] == "only_selected"
            and not sensor.entity_description.is_selected_entity
        ):
            continue
        selected_sensors.append(sensor)

    async_add_entities(selected_sensors)
