#include <simgrid/s4u.hpp>
#include <xbt/log.h>

namespace sg4 = simgrid::s4u;

XBT_LOG_NEW_DEFAULT_CATEGORY(pipeline, "default pipeline log");

// Constants
const int NUM_TASKS = 10;

// Task structure
struct Task {
  int id;
  std::string data;
};

// --------------------------------------------------------------------------
// Stage 1: Initial Processing
// --------------------------------------------------------------------------
void stage1_worker() {
  auto stage1_mailbox = sg4::Mailbox::by_name("Stage1");
  auto stage2_mailbox = sg4::Mailbox::by_name("Stage2");

  while (true) {
    Task *task = stage1_mailbox->get<Task>();
    if (task->id == -1) {
      delete task;
      stage2_mailbox->put(new Task{-1, ""}, 0);
      XBT_INFO("[%s] Forwarded poison pill to Stage2",
               sg4::this_actor::get_cname());
      break;
    }

    // Log with actor name and simulation time
    XBT_INFO("[%s] Processing task %d (t=%.3f)", sg4::this_actor::get_cname(),
             task->id, sg4::Engine::get_clock());
    sg4::this_actor::execute(1e9); // 1 second work

    stage2_mailbox->put(task, 0);
    XBT_INFO("[%s] Sent task %d to Stage2 (t=%.3f)",
             sg4::this_actor::get_cname(), task->id, sg4::Engine::get_clock());
  }
}

// --------------------------------------------------------------------------
// Stage 2: Intermediate Processing
// --------------------------------------------------------------------------
void stage2_worker() {
  auto stage2_mailbox = sg4::Mailbox::by_name("Stage2");
  auto stage3_mailbox = sg4::Mailbox::by_name("Stage3");

  while (true) {
    Task *task = stage2_mailbox->get<Task>();
    if (task->id == -1) {
      delete task;
      stage3_mailbox->put(new Task{-1, ""}, 0);
      XBT_INFO("[%s] Forwarded poison pill to Stage3",
               sg4::this_actor::get_cname());
      break;
    }

    XBT_INFO("[%s] Processing task %d (t=%.3f)", sg4::this_actor::get_cname(),
             task->id, sg4::Engine::get_clock());
    sg4::this_actor::execute(1e9);

    stage3_mailbox->put(task, 0);
    XBT_INFO("[%s] Sent task %d to Stage3 (t=%.3f)",
             sg4::this_actor::get_cname(), task->id, sg4::Engine::get_clock());
  }
}

// --------------------------------------------------------------------------
// Stage 3: Final Processing
// --------------------------------------------------------------------------
void stage3_worker() {
  auto stage3_mailbox = sg4::Mailbox::by_name("Stage3");

  while (true) {
    Task *task = stage3_mailbox->get<Task>();
    if (task->id == -1) {
      delete task;
      XBT_INFO("[%s] Received termination signal",
               sg4::this_actor::get_cname());
      break;
    }

    XBT_INFO("[%s] Finalizing task %d (t=%.3f)", sg4::this_actor::get_cname(),
             task->id, sg4::Engine::get_clock());
    sg4::this_actor::execute(1e9);

    delete task;
  }
}

// --------------------------------------------------------------------------
// Source Actor
// --------------------------------------------------------------------------
void source_actor() {
  auto stage1_mailbox = sg4::Mailbox::by_name("Stage1");

  for (int i = 0; i < NUM_TASKS; ++i) {
    Task *task = new Task{i, "Initial-Data"};
    stage1_mailbox->put(task, 0);
    XBT_INFO("[%s] Sent task %d to Stage1 (t=%.3f)",
             sg4::this_actor::get_cname(), i, sg4::Engine::get_clock());
  }

  stage1_mailbox->put(new Task{-1, ""}, 0);
  XBT_INFO("[%s] Sent poison pill to Stage1", sg4::this_actor::get_cname());
}

// --------------------------------------------------------------------------
// Main
// --------------------------------------------------------------------------
int main(int argc, char **argv) {
  sg4::Engine e(&argc, argv);
  e.load_platform("platform.xml");

  // Create actors
  sg4::Actor::create("Source", e.host_by_name("Host1"), source_actor);
  sg4::Actor::create("Stage1", e.host_by_name("Host1"), stage1_worker);
  sg4::Actor::create("Stage2", e.host_by_name("Host1"), stage2_worker);
  sg4::Actor::create("Stage3", e.host_by_name("Host1"), stage3_worker);

  e.run();
  return 0;
}
