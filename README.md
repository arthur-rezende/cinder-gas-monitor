# cinder-gas-monitor
O Projeto Cinder entrega um sistema embarcado, arquitetado sobre o microcontrolador ESP32, que opera como um agente autônomo de mitigação de riscos. O sistema é estruturado em quatro atividades:
Leitura ininterrupta utilizando o sensor MQ-4 (calibrado para metano de 200 a 10.000 ppm) e MQ-135 (qualidade do ar e gases tóxicos de 10 a 1.000 ppm). Esses dados são correlacionados com o monitoramento térmico (DS18B20).
Em vez de reagir apenas a tetos fixos, o sistema processa a taxa de variação (velocidade de aumento) da concentração de gás e, com base nisso, categoriza a atmosfera dinamicamente em três níveis (Normal, Atenção e Perigo).
Ao detectar a violação dos limiares de segurança, o hardware reage em milissegundos. Além do alarme local (sinalização visual e buzzer), o sistema energiza imediatamente a ventilação forçada (cooler) para dispersar ativamente o bolsão de gás e quebrar a cadeia de explosividade/toxicidade do ambiente.
Transmissão contínua do estado do sistema via protocolo MQTT, permitindo integração direta com dashboards para gestores remotos. Localmente, o sistema conta com redundância de hardware em um painel físico (enclosure), oferecendo intervenção manual de pânico (exaustão imediata) e supressão temporária de alarmes sonoros.

