#ifndef UPDATE_ACTION_H
#define UPDATE_ACTION_H

class UpdateAction
{
public:
    enum Action {
        InvalidAction,
        ScrollUp,
        ScrollDown,
        LinesInserted,
        LinesRemoved
    };

    UpdateAction(Action action, int from_line, int count)
        : action(action)
        , from_line(from_line)
        , count(count)
    { }

    UpdateAction(Action action, int count)
        : action(action)
        , from_line(0)
        , count(count)
    { }

    Action action;
    int from_line;
    int count;
};

#endif // UPDATE_ACTION_H
