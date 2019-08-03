

Description:
struct ai_task *task

A task is normally generated from some task source(see struct task_src), and could be pass to different task transformers( see struct task_transformer),
and then finally consumed by different task_consumers(see struct task_consumer).
Not all the task could get correct transformers when they created by the task source, and most of time a transformer is not only dedicated to one task, in fact,
most of transforms are created at the beginning, and wait different task source to send tasks.
But the task consumers are really designed and created for the special task, after finish the task, we should release task consumers. We allow that a task can
have many task consumers, and the task will be passed to those consumers one by one. Application or users(in fact, it's our main thread) will have responsibility to
create taks consumers and link the task consumers to the task. After all consumers execute, the task will be freed.

Now at least we support 4 tasks:
1, multimedia stream

2, chat

3, smart home control and status query

4, text to speech
