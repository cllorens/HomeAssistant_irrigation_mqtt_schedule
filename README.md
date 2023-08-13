# HomeAssistant irrigation system based in the websocket api and the MQTT service
Create irrigation schedules in json format in different mqtt topics.
when you click publish in the webpage it send a mqtt publish through the websocket api.

to lisen to the mqtt messages it use a automation yaml that must be included in the packes folder:

``` yaml
# Configuration text for the schedules
input_text:
   shedule0:
    name: shedule0
    max: 255
   shedule1:
    name: shedule1
    max: 255
   shedule2:
    name: shedule2
    max: 255
   shedule3:
    name: shedule3
    max: 255
   shedule4:
    name: shedule4
    max: 255
   shedule5:
    name: shedule5
    max: 255
   shedule6:
    name: shedule6
    max: 255
   shedule7:
    name: shedule7
    max: 255
   shedule8:
    name: shedule8
    max: 255
   shedule9:
    name: shedule9
    max: 255
    
# On Change in topic modify topics
automation:
  - alias: Set schedule0
    trigger:
      platform: mqtt
      topic: 'irrigation/esclavas/schedule0'
    action:
      service: input_text.set_value
      data_template:
        entity_id: input_text.shedule0
        value: "{{ trigger.payload }}"
  - alias: Set schedule1
    trigger:
      platform: mqtt
      topic: 'irrigation/esclavas/schedule1'
    action:
      service: input_text.set_value
      data_template:
        entity_id: input_text.shedule1
        value: "{{ trigger.payload }}"
  - alias: Set schedule2
    trigger:
      platform: mqtt
      topic: 'irrigation/esclavas/schedule2'
    action:
      service: input_text.set_value
      data_template:
        entity_id: input_text.shedule2
        value: "{{ trigger.payload }}"
  - alias: Set schedule3
    trigger:
      platform: mqtt
      topic: 'irrigation/esclavas/schedule3'
    action:
      service: input_text.set_value
      data_template:
        entity_id: input_text.shedule3
        value: "{{ trigger.payload }}"
  - alias: Set schedule4
    trigger:
      platform: mqtt
      topic: 'irrigation/esclavas/schedule4'
    action:
      service: input_text.set_value
      data_template:
        entity_id: input_text.shedule4
        value: "{{ trigger.payload }}"
  - alias: Set schedule5
    trigger:
      platform: mqtt
      topic: 'irrigation/esclavas/schedule5'
    action:
      service: input_text.set_value
      data_template:
        entity_id: input_text.shedule5
        value: "{{ trigger.payload }}"
  - alias: Set schedule6
    trigger:
      platform: mqtt
      topic: 'irrigation/esclavas/schedule6'
    action:
      service: input_text.set_value
      data_template:
        entity_id: input_text.shedule6
        value: "{{ trigger.payload }}"
  - alias: Set schedule7
    trigger:
      platform: mqtt
      topic: 'irrigation/esclavas/schedule7'
    action:
      service: input_text.set_value
      data_template:
        entity_id: input_text.shedule7
        value: "{{ trigger.payload }}"
  - alias: Set schedule8
    trigger:
      platform: mqtt
      topic: 'irrigation/esclavas/schedule8'
    action:
      service: input_text.set_value
      data_template:
        entity_id: input_text.shedule8
        value: "{{ trigger.payload }}"
  - alias: Set schedule9
    trigger:
      platform: mqtt
      topic: 'irrigation/esclavas/schedule9'
    action:
      service: input_text.set_value
      data_template:
        entity_id: input_text.shedule9
        value: "{{ trigger.payload }}"

```

to add control for the vessel tare in home assistant:

```yaml
# Configuration number for the tare
input_number:
   vessel_tare:
    name: Tara vasija
    min: 30000
    max: 35000
    step: 0.1
    unit_of_measurement: kg
    icon: mdi:scale-balance

# On Change in topic modify number
automation:
  - alias: Set vessel tare
    trigger:
      platform: mqtt
      topic: 'irrigation/esclavas/tare'
    action:
      service: input_number.set_value
      data_template:
        entity_id: input_number.vessel_tare
        value: "{{ trigger.payload }}"

# On modify number modify topic
  - alias: Vessel tare moved
    trigger:
      platform: state
      entity_id: input_number.vessel_tare
    action:
      service: mqtt.publish
      data_template:
        topic: 'irrigation/esclavas/tare'
        retain: true
        payload: "{{ states('input_number.vessel_tare') | int }}"
```

to add control to the vessel max value in orde to calc the rgb color for the leds in the m5stack:

```yaml
# Configuration number for the vessel max vallue
input_number:
   vessel_max:
    name: Max vasija
    min: 1
    max: 100
    step: 1
    unit_of_measurement: l
    icon: mdi:scale-balance

# On Change in topic modify number
automation:
  - alias: Set vessel max
    trigger:
      platform: mqtt
      topic: 'irrigation/esclavas/max'
    action:
      service: input_number.set_value
      data_template:
        entity_id: input_number.vessel_max
        value: "{{ trigger.payload }}"

# On modify number modify topic
  - alias: Vessel max moved
    trigger:
      platform: state
      entity_id: input_number.vessel_max
    action:
      service: mqtt.publish
      data_template:
        topic: 'irrigation/esclavas/max'
        retain: true
        payload: "{{ states('input_number.vessel_max') | int }}"
```

you need to include this in the configuration.yaml to load all yamls in packages folder:
```yaml
homeassistant:
  packages: !include_dir_named packages/
```
