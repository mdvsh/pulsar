//
// Created by Madhav Shekhar Sharma on 4/16/24.
//

#ifndef PULSAR_SRC_ENGINE_CORE_EVENT_H_
#define PULSAR_SRC_ENGINE_CORE_EVENT_H_

class Event {
 public:
  enum EventType {
    QUIT,
    NOTIFY,
    WARN,
  };

  void setEvent(EventType event_type) { m_event_type = event_type; }


 private:
  EventType m_event_type;
};

#endif  // PULSAR_SRC_ENGINE_CORE_EVENT_H_
