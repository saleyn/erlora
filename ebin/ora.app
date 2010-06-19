{application, ora,
   [{description, "ORACLE Port"},
    {vsn, "1.0"},
    {modules, [ora_app, ora]},
    {registered, []},
    {applications, [kernel, stdlib]},
    {mod, {ora_app, [{debug, false}]}}
   ]
}.

