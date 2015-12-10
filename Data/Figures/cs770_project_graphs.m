groups = {'Movement', 'Rotation'};
legend_labels = {'Joystick', 'Kinesiological', 'Kinesiological+'};

% Score
clear measure std_error title fig

measure = [0.85 , 0.89 , 0.88; 0.75 , 0.55 , 0.82];
std_err = [0.02 , 0.02 , 0.02; 0.06 , 0.07 , 0.04];

title = 'Score - Control Method x Task Type';
fig = BarGraphWithError(measure, std_err, groups, legend_labels, title);

print(fig, title, '-dsvg', '-cmyk');
close(fig);

% Usefulness
clear measure std_error title fig

measure = [5.5 , 5.33 , 5.5; 5.25 , 4.67 , 5.58];
std_err = [0.44 , 0.45 , 0.17; 0.44 , 0.68 , 0.28];

title = 'Usefulness - Control Method x Task Type';
fig = BarGraphWithError(measure, std_err, groups, legend_labels, title);

print(fig, title, '-dsvg', '-cmyk');
close(fig);

% Enjoyment
clear measure std_error title fig

measure = [5.58 , 5.33 , 5.67; 5.08 , 5.00 , 5.25];
std_err = [0.28 , 0.19 , 0.65; 0.21 , 0.24 , 0.74];

title = 'Enjoyment - Control Method x Task Type';
fig = BarGraphWithError(measure, std_err, groups, legend_labels, title);

print(fig, title, '-dsvg', '-cmyk');
close(fig);

% Ease of Use
clear measure std_error title fig

measure = [5.00 , 5.08 , 4.75; 4.67 , 2.92 , 3.92];
std_err = [0.49 , 0.25 , 0.48; 0.24 , 0.28 , 0.42];

title = 'Ease of Use - Control Method x Task Type';
fig = BarGraphWithError(measure, std_err, groups, legend_labels, title);

print(fig, title, '-dsvg', '-cmyk');
close(fig);