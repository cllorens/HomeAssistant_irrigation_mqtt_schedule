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

you need to include this in the configuration.yaml to load all yamls in packages folder:
```yaml
homeassistant:
  packages: !include_dir_named packages/
```
