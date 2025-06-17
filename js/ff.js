/* ff.js
 * Default JavaScript file for https://forwarderfactory.com
 * Licensed under the MIT license
 * Copyright (c) 2025 Jacob Nilsson
 */

function is_phone() {
    return window.innerWidth <= 600;
}

function include(file) {
    const script = document.createElement('script');
    script.src = file;
    script.type = 'text/javascript';
    script.defer = true;

    document.getElementsByTagName('head').item(0).appendChild(script);
}

function get_cookie(name) {
    const name_eq = `${name}=`;
    const ca = document.cookie.split(';');
    for (let i = 0; i < ca.length; i++) {
        let c = ca[i];
        while (c.charAt(0) === ' ') {
            c = c.substring(1, c.length);
        }
        if (c.indexOf(name_eq) === 0) {
            return c.substring(name_eq.length, c.length);
        }
    }
    return null;
}

function get_path() {
    return window.location.pathname;
}

function set_path(path) {
    window.history.pushState({}, '', path);
}

function cookie_exists(name) {
    return get_cookie(name) !== null;
}

function WSCBackgroundRepeatingSpawner(speed = 0.5, creation_interval = 8000) {
    const spawnCount = 10;
    const horizontalSpacing = 400;

    if (WSCBackgroundRepeatingSpawner.intervalId) {
        clearInterval(WSCBackgroundRepeatingSpawner.intervalId);
        WSCBackgroundRepeatingSpawner.intervalId = null;
    }

    const oldContainer = document.getElementById('background-spawner-container');
    if (oldContainer) {
        oldContainer.remove();
    }

    const container = document.createElement('div');
    container.id = 'background-spawner-container';
    Object.assign(container.style, {
        position: 'fixed',
        top: '0',
        left: '0',
        width: '100vw',
        height: '100vh',
        pointerEvents: 'none',
        overflow: 'hidden',
        zIndex: '-9999',
    });
    document.body.appendChild(container);

    document.body.style.overflow = 'hidden';

    const cachedImageSrc = '/img/logo.svg';
    //const cachedImageSrc = '/img/background-logo-1.png';

    function createImage(initialX, initialY) {
        const img = document.createElement('img');
        img.className = 'fwf-image';
        img.src = cachedImageSrc;
        img.style.position = 'absolute';
        img.style.top = '0';
        img.style.right = '0';
        img.style.opacity = '0.2';
        img.style.userSelect = 'none';
        img.draggable = false;
        img.style.filter = `hue-rotate(${Math.random() * 360}deg)`;
        img.style.width = '75px';
        img.style.height = 'auto';

        container.appendChild(img);

        let x = initialX;
        let y = initialY;

        function animate() {
            x += speed;
            y += speed;

            img.style.transform = `translate(${-x}px, ${y}px)`;

            if (y > window.innerHeight || x > window.innerWidth + 200) {
                img.remove();
                return;
            }
            requestAnimationFrame(animate);
        }

        animate();
    }

    function spawnImages(topOffset, rightOffset) {
        const colCount = Math.floor((window.innerWidth / horizontalSpacing) * 2);
        for (let col = 0; col < colCount; col++) {
            const initialX = rightOffset + col * horizontalSpacing;
            const initialY = topOffset;
            createImage(initialX, initialY);
        }
    }

    for (let i = 0; i < spawnCount; i++) {
        const topOffset = -200 + (i * speed * creation_interval) / 30;
        const rightOffset = -500 + (i * speed * creation_interval) / 30;
        spawnImages(topOffset, rightOffset);
    }

    WSCBackgroundRepeatingSpawner.intervalId = setInterval(() => {
        spawnImages(-200, -500);
    }, creation_interval);

    if (WSCBackgroundRepeatingSpawner.resizeTimeout) {
        clearTimeout(WSCBackgroundRepeatingSpawner.resizeTimeout);
    }
    window.onresize = () => {
        clearTimeout(WSCBackgroundRepeatingSpawner.resizeTimeout);
        WSCBackgroundRepeatingSpawner.resizeTimeout = setTimeout(() => {
            // Remove container and re-enable scrolling before restart
            if (WSCBackgroundRepeatingSpawner.intervalId) {
                clearInterval(WSCBackgroundRepeatingSpawner.intervalId);
                WSCBackgroundRepeatingSpawner.intervalId = null;
            }
            container.remove();
            document.body.style.overflow = ''; // restore scroll

            WSCBackgroundRepeatingSpawner(speed, creation_interval);
        }, 200);
    };

    document.addEventListener('visibilitychange', () => {
        if (document.visibilityState === 'visible') {
            if (WSCBackgroundRepeatingSpawner.intervalId) {
                clearInterval(WSCBackgroundRepeatingSpawner.intervalId);
                WSCBackgroundRepeatingSpawner.intervalId = null;
            }
            container.remove();
            document.body.style.overflow = '';
            WSCBackgroundRepeatingSpawner(speed, creation_interval);
        }
    });
}

const click_object = new Audio('/audio/click.wav');
function play_click() {
    click_object.currentTime = 0;
    click_object.play();
}

function hide_all_windows() {
    const windows = document.getElementsByClassName('floating_window');
    for (let i = 0; i < windows.length; i++) {
        windows[i].style.display = 'none';
        while (windows[i].firstChild) {
            windows[i].removeChild(windows[i].firstChild);
        }
        windows[i].remove();
    }

    const grids = document.getElementsByClassName('grid');
    for (let i = 0; i < grids.length; i++) {
        grids[i].style.display = 'flex';
    }

    // hide #browse-search and #browse-filter-button if they exist
    const search = document.getElementById('sandbox-search');
    if (search) {
        search.remove();
    }
    const filter = document.getElementById('sandbox-filter-button');
    if (filter) {
        filter.remove();
    }

    // hide #browse-search and #browse-filter-button if they exist
    const browse_search = document.getElementById('browse-search');
    if (browse_search) {
        browse_search.remove();
    }
    const browse_filter = document.getElementById('browse-filter-button');
    if (browse_filter) {
        browse_filter.remove();
    }

    // show title if hidden
    const title = document.getElementById('page-header');
    if (title) {
        title.style.display = 'block';
    }

    // hide blacken
    const blacken = document.getElementById('blacken');
    if (blacken) {
        blacken.remove();
    }

    set_path("/");
}

function hide_initial() {
    const windows = document.getElementsByClassName('floating_window');
    for (let i = 0; i < windows.length; i++) {
        windows[i].style.display = 'none';
        while (windows[i].firstChild) {
            windows[i].removeChild(windows[i].firstChild);
        }
    }

    const grids = document.getElementsByClassName('grid');
    for (let i = 0; i < grids.length; i++) {
        grids[i].style.display = 'none';
    }

    const search = document.getElementById('sandbox-search');
    if (search) {
        search.style.display = 'none';
    }
    const filter = document.getElementById('sandbox-filter-button');
    if (filter) {
        filter.style.display = 'none';
    }
    const browse_search = document.getElementById('browse-search');
    if (browse_search) {
        browse_search.style.display = 'none';
    }
    const browse_filter = document.getElementById('browse-filter-button');
    if (browse_filter) {
        browse_filter.style.display = 'none';
    }

    const title = document.getElementById('page-header');
    if (title) {
        title.style.display = 'block';
    }

    const blacken = document.getElementById('blacken');
    if (blacken) {
        blacken.style.display = 'none';
    }
}

class WindowProperties {
    constructor({
        classes = [],
        close_button = true,
        moveable = false,
        close_on_escape = true,
        remove_existing = true,
        function_on_close = null
    } = {}) {
        this.classes = classes;
        this.close_button = close_button;
        this.moveable = moveable;
        this.close_on_escape = close_on_escape;
        this.remove_existing = remove_existing;
        this.function_on_close = function_on_close;
    }
}

function create_window(id, prop = new WindowProperties()){
    if (prop.remove_existing) {
        const windows = document.getElementsByClassName('floating_window');
        for (let i = 0; i < windows.length; i++) {
            while (windows[i].firstChild) {
                windows[i].removeChild(windows[i].firstChild);
            }
            windows[i].remove();
        }
    }
    // remove existing with same id always
    const existing = document.getElementById(id);
    if (existing) {
        existing.style.display = 'none';
        while (existing.firstChild) {
            existing.removeChild(existing.firstChild);
        }
    }
    const window = document.createElement('div');
    window.className = 'floating_window';

    if (prop.classes && prop.classes.length > 0) {
        for (const cls of prop.classes) {
            window.classList.add(cls);
        }
    }

    window.id = id;
    /*
    if (prop.close_on_click_outside) {
        window.onclick = (event) => {
            if (event.target === window) {
                if (prop.function_on_close) {
                    prop.function_on_close();
                    return;
                }
                //hide_all_windows();
                window.remove();
                const windows = document.getElementsByClassName('floating_window');
                if (windows.length === 0) {
                    hide_all_windows();
                }
            }
        }
    }
    */

    if (prop.close_on_escape) {
        document.onkeydown = (event) => {
            if (event.key === 'Escape') {
                if (prop.function_on_close) {
                    prop.function_on_close();
                    return;
                }

                window.remove();
                const windows = document.getElementsByClassName('floating_window');
                if (windows.length === 0) {
                    hide_all_windows();
                }
            }
        }
    }

    window.oncontextmenu = (event) => {
        event.preventDefault();
    }

    let xpos = 0;
    let ypos = 0;
    let offsetX = 0;
    let offsetY = 0;

    if (prop.moveable) {
        window.onmousedown = (event) => {
            xpos = event.clientX;
            ypos = event.clientY;
            offsetX = xpos - window.offsetLeft;
            offsetY = ypos - window.offsetTop;

            document.onmousemove = (event) => {
                window.style.left = (event.clientX - offsetX) + 'px';
                window.style.top = (event.clientY - offsetY) + 'px';
            }
        }

        document.onmouseup = () => {
            document.onmousemove = null;
        }
    }

    if (prop.close_button) {
        const close = document.createElement('a');
        close.innerHTML = '✕';
        close.id = 'window-close';
        close.style.position = 'fixed';
        close.style.padding = '10px';
        close.style.top = '0';
        close.style.right = '0';
        close.style.textDecoration = 'none';
        close.style.color = 'black';
        close.onclick = () => {
            play_click();
            if (prop.function_on_close) {
                prop.function_on_close();
                return;
            }
            window.remove();
            const windows = document.getElementsByClassName('floating_window');
            if (windows.length === 0) {
                hide_all_windows();
            }
        }

        window.appendChild(close);
    }

    document.body.appendChild(window);

    return window;
}

function show_terms() {
    play_click();
    const terms = create_window('tos-window');

    const title = document.createElement('h1');
    title.innerHTML = 'Terms of Service';

    const paragraph = document.createElement('p');
    paragraph.innerHTML = 'Loading terms...';

    terms.appendChild(title);
    terms.appendChild(paragraph);

    fetch('https://raw.githubusercontent.com/ForwarderFactory/documents/refs/heads/master/terms-of-service.txt')
        .then(response => {
            if (!response.ok) {
                throw new Error('network response was not ok.');
            }
            return response.text();
        })
        .then(text => {
            paragraph.innerHTML = text.replace(/\n/g, '<br>');
        })
        .catch(error => {
            paragraph.innerHTML = 'failed to load terms of service.';
            console.error('error fetching terms:', error);
        });
}

function show_login(_error = "") {
    play_click();
    set_path('/login');

    hide_initial();

    /* username/email, password */
    let ret = {};

    const submit_data = () => {
        play_click();

        const json = {
            password: ret.password.value,
        };
        if (ret.username.value.includes('@')) {
            json.email = ret.username.value;
        } else {
            json.username = ret.username.value;
        }

        fetch("/api/try_login", {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(json),
        })
            .then(response => response.json())
            .then(json => {
                if (json.error_str) {
                    show_login(json.error_str);
                    return;
                } else if (json.key) {
                    const login = create_window('login-window');
                    const title = document.createElement('h1');

                    title.innerHTML = 'Stay logged in?';
                    title.className = 'floating_window_title';

                    const paragraph = document.createElement('p');
                    paragraph.innerHTML = 'Do you want to stay logged in? This will allow you to access your account without having to log in again for 30 days, or until Forwarder Factory is updated. Note that this may pose a significant security risk if you are using a shared or public computer.';
                    paragraph.className = 'floating_window_paragraph';
                    paragraph.id = 'login-window-paragraph';

                    const yes_button = document.createElement('button');
                    yes_button.innerHTML = 'Sure!';
                    yes_button.className = 'login-button';
                    yes_button.onclick = () => {
                        // just send an empty request to /api/stay_logged_in
                        fetch('/api/stay_logged_in', {
                            method: 'POST',
                            headers: {
                                'Content-Type': 'application/json',
                            },
                            body: JSON.stringify({ key: json.key }),
                        })
                            .then(() => {
                                hide_all_windows();
                                window.location.reload();
                            })
                            .catch((error) => {
                                console.error('Error:', error);
                            });
                    }
                    const no_button = document.createElement('button');
                    no_button.innerHTML = 'No thanks';
                    no_button.className = 'login-button';
                    no_button.onclick = () => {
                        window.location.href = '/';
                    }
                    no_button.style.marginLeft = '10px';
                    no_button.style.marginRight = '10px';

                    login.appendChild(title);
                    login.appendChild(paragraph);
                    login.appendChild(yes_button);
                    login.appendChild(no_button);

                    return;
                }

                throw new Error('Invalid response from server: ' + JSON.stringify(json));
            })
            .catch((error) => {
                console.error('Error:', error);
            });
    }

    const ask_for_password = () => {
        play_click();

        const login = create_window('login-window');

        const password = document.createElement('input');
        password.type = 'password';
        password.name = 'password';
        password.placeholder = 'Password';
        password.className = 'login-input';
        password.id = 'login-password';
        password.onclick = () => {
            play_click();
            const errors = document.getElementsByClassName('error');
            for (let i = 0; i < errors.length; i++) {
                const error = errors[i];
                if (error.id === 'login-error') {
                    login.removeChild(error);
                }
            }
        }
        if (ret.password !== undefined && ret.password.value !== undefined) {
            password.value = ret.password.value;
        }

        const title = document.createElement('h1');
        title.innerHTML = "Welcome " + ret.username.value + "!";

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please enter your password!';

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'login-button';
        back.onclick = () => {
            ret.password = password;
            ask_for_user();
        }
        back.style.marginRight = '10px';

        const submit = document.createElement('button');
        submit.innerHTML = 'Login';
        submit.className = 'login-button';
        submit.onclick = () => {
            ret.password = password;
            submit_data();
        }

        login.appendChild(title);
        login.appendChild(paragraph);
        login.appendChild(password);
        login.appendChild(document.createElement('br'));
        login.appendChild(document.createElement('br'));
        login.appendChild(back);
        login.appendChild(submit);
    }

    const ask_for_user = () => {
        const login = create_window('login-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Login';
        title.className = 'floating_window_title';
        title.id = 'login-window-title';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Welcome back to Forwarder Factory. Please enter your username or associated email address!';
        paragraph.className = 'floating_window_paragraph';
        paragraph.id = 'login-window-paragraph';

        login.appendChild(title);
        login.appendChild(paragraph);

        const username = document.createElement('input');
        username.type = 'text';
        username.name = 'username';
        username.placeholder = 'Username';
        username.className = 'login-input';
        username.id = 'login-username';
        username.onclick = () => {
            play_click();

            const errors = document.getElementsByClassName('error');
            for (let i = 0; i < errors.length; i++) {
                const error = errors[i];
                if (error.id === 'login-error') {
                    login.removeChild(error);
                }
            }
        }
        if (ret.username !== undefined && ret.username.value !== undefined) {
            username.value = ret.username.value;
        }

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'login-button';
        submit.onclick = () => {
            ret.username = username;
            ask_for_password();
        }

        login.appendChild(title);
        login.appendChild(paragraph);
        login.appendChild(username);
        login.appendChild(document.createElement('br'));
        login.appendChild(document.createElement('br'));
        login.appendChild(submit);

        if (_error !== "") {
            const error = document.createElement('p');
            error.innerHTML = _error;
            error.className = 'error';
            error.id = 'login-error';

            const br = document.createElement('br');
            br.className = 'error';

            login.appendChild(br);
            login.appendChild(error);
        }

        const notice = document.createElement('p');
        notice.innerHTML += "By using this service, you agree to our ";

        const terms = document.createElement('a');
        terms.innerHTML = "Terms of Service";
        terms.onclick = () => show_terms();
        terms.style.cursor = 'pointer';
        terms.style.textDecoration = 'underline';

        notice.appendChild(terms);
        login.appendChild(notice);
    }

    ask_for_user();
}

function show_register(_error = ""){
    play_click();
    set_path("/register");

    hide_initial();

    let ret = {};

    const submit_data = () => {
        play_click();

        if (ret.password.value !== ret.confirm.value) {
            show_register('Passwords do not match!');
            return;
        }

        const data = {
            username: ret.username.value,
            password: ret.password.value,
            email: ret.email.value,
        };

        fetch("/api/try_register", {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(data),
        })
            .then(response => {
                if (response.status === 204) { // success without email verification needed
                    hide_all_windows();
                }

                return response.json();
            })
            .then(json => {
                if (json && json.error_str) {
                    show_register(json.error_str);
                    return;
                } else if (json && json.note && json.note === "FF_EMAIL_VERIFICATION_REQUIRED") {
                    const congrats = create_window('congrats-window');

                    const title = document.createElement('h1');
                    title.innerHTML = 'Congratulations!';
                    title.className = 'floating_window_title';
                    title.id = 'congrats-window-title';

                    const paragraph = document.createElement('p');
                    paragraph.innerHTML = 'You have successfully registered! Before you can log in, you must verify your email address. Please check your email for a verification link.';
                    paragraph.className = 'floating_window_paragraph';
                    paragraph.id = 'congrats-window-paragraph';

                    const button = document.createElement('button');
                    button.innerHTML = 'Close';
                    button.className = 'congrats-button';
                    button.onclick = () => {
                        play_click();
                        hide_all_windows();
                    }

                    congrats.appendChild(title);
                    congrats.appendChild(paragraph);
                    congrats.appendChild(button);
                }

                if (json) {
                    throw new Error('Invalid response from server: ' + JSON.stringify(json));
                }
            })
            .catch((error) => {
                console.error('Error:', error);
            });
    }

    const ask_for_confirm = () => {
        play_click();

        const register = create_window('register-window');

        const title = document.createElement('h1');
        title.innerHTML = "Welcome " + ret.username.value + "!";

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please confirm your password!';

        const confirm = document.createElement('input');
        confirm.type = 'password';
        confirm.name = 'password';
        confirm.placeholder = 'Confirm password';
        confirm.className = 'register-input';
        confirm.id = 'register-confirm-password';
        confirm.onclick = () => {
            play_click();
        }
        if (ret.confirm !== undefined && ret.confirm.value !== undefined) {
            confirm.value = ret.confirm.value;
        }

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'register-button';
        submit.onclick = () => {
            ret.confirm = confirm;
            submit_data();
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'register-button';
        back.onclick = () => {
            ret.confirm = confirm;
            ask_for_password();
        }
        back.style.marginRight = '10px';

        register.appendChild(title);
        register.appendChild(paragraph);
        register.appendChild(confirm);
        register.appendChild(document.createElement('br'));
        register.appendChild(document.createElement('br'));
        register.appendChild(back);
        register.appendChild(submit);
    }

    const ask_for_password = () => {
        play_click();

        const register = create_window('register-window');

        const title = document.createElement('h1');
        title.innerHTML = "Welcome " + ret.username.value + "!";

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please enter your password!';

        const password = document.createElement('input');
        password.type = 'password';
        password.name = 'password';
        password.placeholder = 'Password';
        password.className = 'register-input';
        password.id = 'register-password';
        password.onclick = () => {
            play_click();
        }
        if (ret.password !== undefined && ret.password.value !== undefined) {
            password.value = ret.password.value;
        }

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'register-button';
        submit.onclick = () => {
            ret.password = password;
            ask_for_confirm();
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'register-button';
        back.onclick = () => {
            ret.password = password;
            ask_for_username();
        }
        back.style.marginRight = '10px';

        register.appendChild(title);
        register.appendChild(paragraph);
        register.appendChild(password);
        register.appendChild(document.createElement('br'));
        register.appendChild(document.createElement('br'));
        register.appendChild(back);
        register.appendChild(submit);
    }

    const ask_for_username = () => {
        play_click();

        const register = create_window('register-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Register';
        title.className = 'floating_window_title';
        title.id = 'register-window-title';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please enter your desired username!';
        paragraph.className = 'floating_window_paragraph';
        paragraph.id = 'register-window-paragraph';

        const username = document.createElement('input');
        username.type = 'text';
        username.name = 'username';
        username.placeholder = 'Username';
        username.className = 'login-input';
        username.id = 'register-username';
        username.onclick = () => {
            play_click();
        }
        if (ret.username !== undefined && ret.username.value !== undefined) {
            username.value = ret.username.value;
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'register-button';
        back.onclick = () => {
            ret.username = username;
            ask_for_email();
        }
        back.style.marginRight = '10px';

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'register-button';
        submit.onclick = () => {
            ret.username = username;
            ask_for_password();
        }

        register.appendChild(title);
        register.appendChild(paragraph);
        register.appendChild(username);
        register.appendChild(document.createElement('br'));
        register.appendChild(document.createElement('br'));
        register.appendChild(back);
        register.appendChild(submit);
    }

    const ask_for_email = () => {
        play_click();

        const register = create_window('register-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Register';
        title.className = 'floating_window_title';
        title.id = 'register-window-title';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Welcome to Forwarder Factory. Please enter your email address!';
        paragraph.className = 'floating_window_paragraph';
        paragraph.id = 'register-window-paragraph';

        const email = document.createElement('input');
        email.type = 'text';
        email.name = 'username';
        email.placeholder = 'Email address';
        email.className = 'login-input';
        email.id = 'register-email';
        email.onclick = () => {
            play_click();

            const errors = document.getElementsByClassName('error');
            for (let i = 0; i < errors.length; i++) {
                const error = errors[i];
                if (error.id === 'register-error') {
                    register.removeChild(error);
                }
            }
        }
        if (ret.email !== undefined && ret.email.value !== undefined) {
            email.value = ret.email.value;
        }

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'register-button';
        submit.onclick = () => {
            ret.email = email;
            ask_for_username();
        }

        register.appendChild(title);
        register.appendChild(paragraph);
        register.appendChild(email);
        register.appendChild(document.createElement('br'));
        register.appendChild(document.createElement('br'));
        register.appendChild(submit);

        if (_error !== "") {
            const error = document.createElement('p');
            error.innerHTML = _error;
            error.className = 'error';
            error.id = 'register-error';

            const br = document.createElement('br');
            br.className = 'error';

            register.appendChild(br);
            register.appendChild(error);
        }

        const notice = document.createElement('p');
        notice.innerHTML += "By using this service, you agree to our ";

        const terms = document.createElement('a');
        terms.innerHTML = "Terms of Service";
        terms.onclick = () => show_terms();
        terms.style.cursor = 'pointer';
        terms.style.textDecoration = 'underline';

        notice.appendChild(terms);
        register.appendChild(notice);
    }

    ask_for_email();
}

function is_logged_in() {
    return cookie_exists('username');
}

function print_beta() {
    const text = document.createTextNode('Beta');
    const span = document.createElement('span');
    span.appendChild(text);
    span.className = 'beta-watermark';
    span.id = 'beta-watermark';
    span.style.position = 'absolute';
    span.style.top = '0';
    span.style.left = '0';
    span.style.padding = '10px';
    span.style.userSelect = 'none';

    document.body.appendChild(span);
}

function show_discord() {
    const url = "https://discord.gg/TuDcKUdqDS";

    play_click();

    const w = create_window('discord-window');

    const logo = document.createElement('img');
    logo.src = '/img/discord.svg';
    logo.style.width = '50px';
    logo.style.height = '50px';

    const title = document.createElement('h1');
    title.innerHTML = 'Join our Discord server!';
    title.className = 'floating_window_title';

    const paragraph = document.createElement('p');
    paragraph.innerHTML = "Join our Discord server to chat with other users, get help, share cool things, and more."
    paragraph.innerHTML += "<br/><br/><small>Content shared on Discord does not necessarily reflect the views of Forwarder Factory. Please be aware of Discord's Terms of Service before using the service.</small>";

    const button = document.createElement('button');
    button.innerHTML = 'Join Discord';
    button.onclick = () => {
        window.open(url, '_blank');
        hide_all_windows();
    }

    w.appendChild(logo);
    w.appendChild(title);
    w.appendChild(paragraph);
    w.appendChild(button);
}

function get_announcement(title, author, date, text) {
    play_click();

    const w = create_window('announcement-window');
    const title_element = document.createElement('h1');

    title_element.innerHTML = title;
    title_element.className = 'floating_window_title';

    const date_element = document.createElement('em');
    const _date = new Date(date);
    date_element.innerHTML = author + " - " + _date.toLocaleString();
    date_element.className = 'floating_window_paragraph';

    const text_element = document.createElement('p');
    text_element.innerHTML = text;
    text_element.className = 'floating_window_paragraph';

    w.appendChild(title_element);
    w.appendChild(date_element);
    w.appendChild(text_element);

    document.body.appendChild(w);
}

function get_announcements() {
    play_click();

    const get_json = async () => {
        const url = '/api/get_announcements';

        try {
            const response = await fetch(url, {
                method: 'GET',
                headers: {
                    'Content-Type': 'application/json',
                },
            });

            if (response.status === 204) {
                return null;
            }

            return await response.json();
        } catch (error) {
            console.error('Error:', error);
            throw error;
        }
    }

    const w = create_window('announcements-window');

    const title = document.createElement('h1');
    title.innerHTML = 'Announcements';
    title.className = 'floating_window_title';

    w.appendChild(title);

    const is_admin = get_cookie('user_type') === '1';

    get_json().then(data => {
       if (!data || !data.announcements || data.announcements.length === 0) {
           const description = document.createElement('description');
           description.innerHTML = 'No announcements available. Come back later!';
           description.className = 'floating_window_paragraph';
           w.appendChild(document.createElement('br'));
           w.appendChild(document.createElement('br'));
           w.appendChild(description);
           return;
       }

       if (!data) {
           throw new Error('Invalid response from server: ' + JSON.stringify(data));
       }

       let announcements = data.announcements;
       announcements = announcements.map((announcement, index) => ({
           ...announcement,
           index,
       }));

       announcements.sort((a, b) => b.publish_timestamp - a.publish_timestamp);
       announcements.forEach(announcement => {
           const div = document.createElement('div');
           div.className = 'announcement_div';
           div.id = "announcement-" + announcement.index;
           div.style.marginBottom = '10px';
           div.style.padding = '10px';
           div.style.borderRadius = '10px';
           div.onclick = () => {
               get_announcement(announcement.title, announcement.author, announcement.publish_timestamp, announcement.text_html);
           }
           div.onmouseover = () => {
               div.style.scale = '1.05';
           }
           div.onmouseleave = () => {
               div.style.scale = '1';
           }

           if (!announcement.text_html) {
               return;
           }

           if (is_admin) {
                const edit_button = document.createElement('button');
                edit_button.innerHTML = 'Edit';
                edit_button.className = 'edit-announcement-button';
                edit_button.onclick = (event) => {
                    play_click();
                    event.stopPropagation();
                    const create = create_window('edit-announcement-window');

                    const title = document.createElement('h1');
                    title.innerHTML = 'Edit announcement';
                    title.className = 'floating_window_title';

                    const paragraph = document.createElement('p');
                    paragraph.innerHTML = 'Please edit the title and text for the announcement.';

                    const title_input = document.createElement('input');
                    title_input.type = 'text';
                    title_input.name = 'title';
                    title_input.placeholder = 'Title';
                    title_input.className = 'announcement-input';
                    title_input.id = 'announcement-title';
                    title_input.value = announcement.title;

                    const text_input = document.createElement('textarea');
                    text_input.name = 'text';
                    text_input.placeholder = 'Text (markdown)';
                    text_input.className = 'announcement-input';
                    text_input.id = 'announcement-text';
                    text_input.style.height = '200px';
                    text_input.style.width = '80%';
                    text_input.value = announcement.text_markdown;

                    const submit_button = document.createElement('button');
                    submit_button.innerHTML = 'Submit';
                    submit_button.className = 'create-announcement-button';
                    submit_button.onclick = () => {
                        play_click();
                        const json = {
                            announcement_id: announcement.index,
                            title: title_input.value,
                            text_markdown: text_input.value,
                        };

                        fetch('/api/edit_announcement', {
                            method: 'POST',
                            headers: {
                                'Content-Type': 'application/json',
                            },
                            body: JSON.stringify(json),
                        })
                            .then(() => {
                                if (json.error_str) {
                                    show_register(json.error_str);
                                    return;
                                }
                                hide_all_windows();
                                get_announcements();
                            })
                            .catch((error) => {
                                console.error('Error:', error);
                            });
                    }

                    create.appendChild(title);
                    create.appendChild(paragraph);
                    create.appendChild(document.createElement('br'));
                    create.appendChild(document.createElement('br'));
                    create.appendChild(title_input);
                    create.appendChild(document.createElement('br'));
                    create.appendChild(document.createElement('br'));
                    create.appendChild(text_input);
                    create.appendChild(document.createElement('br'));
                    create.appendChild(document.createElement('br'));
                    create.appendChild(submit_button);

                    document.body.appendChild(create);
                }
                div.appendChild(edit_button);

                const delete_button = document.createElement('button');
                delete_button.innerHTML = 'Delete';
                delete_button.className = 'delete-announcement-button';
                delete_button.onclick = (event) => {
                    play_click();
                    event.stopPropagation();
                    fetch('/api/delete_announcement', {
                        method: 'POST',
                        headers: {
                            'Content-Type': 'application/json',
                        },
                        body: JSON.stringify({ announcement_id: announcement.index }),
                    })
                        .then(() => {
                            const announcement_div = document.getElementById('announcement-' + announcement.index);
                            if (announcement_div) {
                                announcement_div.remove();
                            }
                        })
                        .catch((error) => {
                            console.error('Error:', error);
                        });
                }

                div.appendChild(delete_button);
           }

           if (announcement.title) {
                const title = document.createElement('h1');
                title.innerHTML = announcement.title;
                title.className = 'announcement_title';

                if (announcement.publish_timestamp && announcement.publish_timestamp > 0) {
                    const date = new Date(announcement.publish_timestamp);
                    const date_str = date.toLocaleString();
                    title.innerHTML += ` (${date_str})`;
                }

                div.appendChild(title);

                let shortened_text = announcement.text_html;

                // remove any non p tags
                const parser = new DOMParser();
                const doc = parser.parseFromString(shortened_text, 'text/html');
                const paragraphs = doc.querySelectorAll('p');

                if (paragraphs.length > 0) {
                    shortened_text = '';
                    paragraphs.forEach((p, index) => {
                        if (index > 0) {
                            shortened_text += '<br/>';
                        }
                        shortened_text += p.innerHTML;
                    });
                }
                if (shortened_text.length > 100) {
                    shortened_text = shortened_text.substring(0, 100) + '...';
                }

                const text = document.createElement('p');

                text.innerHTML = shortened_text;
                text.className = 'announcement_text';

                div.appendChild(text);
           }

           w.appendChild(document.createElement('br'));
           w.appendChild(div);
       });
    });


    if (get_cookie('user_type') === '1') {
        const button = document.createElement('button');
        button.innerHTML = 'Create announcement';
        button.className = 'create-announcement-button';
        button.onclick = () => {
            play_click();
            const create = create_window('create-announcement-window');

            const title = document.createElement('h1');
            title.innerHTML = 'Create announcement';
            title.className = 'floating_window_title';

            const paragraph = document.createElement('p');
            paragraph.innerHTML = 'Please enter the title and text for the announcement.';

            const title_input = document.createElement('input');
            title_input.type = 'text';
            title_input.name = 'title';
            title_input.placeholder = 'Title';
            title_input.className = 'announcement-input';
            title_input.id = 'announcement-title';

            const text_input = document.createElement('textarea');
            text_input.name = 'text';
            text_input.placeholder = 'Text (markdown)';
            text_input.className = 'announcement-input';
            text_input.id = 'announcement-text';
            text_input.style.height = '200px';
            text_input.style.width = '80%';

            const submit_button = document.createElement('button');
            submit_button.innerHTML = 'Submit';
            submit_button.className = 'create-announcement-button';
            submit_button.onclick = () => {
                play_click();
                const json = {
                    title: title_input.value,
                    text_markdown: text_input.value,
                };

                fetch('/api/create_announcement', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                    },
                    body: JSON.stringify(json),
                })
                    .then(() => {
                        if (json.error_str) {
                            show_register(json.error_str);
                            return;
                        }
                        hide_all_windows();
                        get_announcements();
                    })
                    .catch((error) => {
                        console.error('Error:', error);
                    });
            }

            create.appendChild(title);
            create.appendChild(paragraph);
            create.appendChild(document.createElement('br'));
            create.appendChild(document.createElement('br'));
            create.appendChild(title_input);
            create.appendChild(document.createElement('br'));
            create.appendChild(document.createElement('br'));
            create.appendChild(text_input);
            create.appendChild(document.createElement('br'));
            create.appendChild(document.createElement('br'));
            create.appendChild(submit_button);

            document.body.appendChild(create);
        }

        w.appendChild(button);
    }

    document.body.appendChild(w);
}

function update_profile(profile, icon = null) {
    play_click();

    const url = '/api/update_profile';

    const form = new FormData();

    form.append('json', new Blob([JSON.stringify(profile)], { type: 'application/json' }));

    if (icon && icon.files && icon.files[0]) {
        form.append('icon', icon.files[0], icon.files[0].name);
    }

    fetch(url, {
        method: 'POST',
        body: form,
    })
        .then(response => {
            if (response.status === 204) {
                return;
            }

            return response.json();
        })
        .catch((error) => {
            console.error('Error:', error);
        });
}

async function edit_profile(username, curr_profile) {
    play_click();

    const win = create_window('edit-profile-window');
    const title = document.createElement('h1');

    title.innerHTML = 'Editing profile for ' + username;
    title.className = 'floating_window_title';
    title.id = 'edit-profile-window-title';

    const display_name_h2 = document.createElement('h2');
    display_name_h2.innerHTML = 'Reassign display name';
    display_name_h2.className = 'floating_window_subtitle';

    const display_name_p = document.createElement('p');
    display_name_p.innerHTML = 'This will change the display name for your profile. If you leave this empty, your username will be used as the display name.';
    display_name_p.className = 'floating_window_paragraph';

    const display_name = document.createElement('input');
    display_name.type = 'text';
    display_name.name = 'display_name';
    display_name.placeholder = 'Display name';
    display_name.className = 'edit-profile-input';
    display_name.id = 'edit-profile-display-name';
    display_name.value = curr_profile.display_name || '';

    const description_h2 = document.createElement('h2');
    description_h2.innerHTML = 'Reassign description';
    description_h2.className = 'floating_window_subtitle';

    const description_p = document.createElement('p');
    description_p.innerHTML = 'This will change the description for your profile. If you leave this empty, no description will be shown.';
    description_p.className = 'floating_window_paragraph';

    const description = document.createElement('textarea');
    description.name = 'description';
    description.placeholder = 'Description (plain text)';
    description.className = 'edit-profile-input';
    description.id = 'edit-profile-description';
    description.style.height = '200px';
    description.style.width = '80%';
    description.value = curr_profile.description || '';

    const img_h2 = document.createElement('h2');
    img_h2.innerHTML = 'Reassign profile icon';
    img_h2.className = 'floating_window_subtitle';

    const img_p = document.createElement('p');
    img_p.innerHTML = 'This will change the profile icon for your profile. If you leave this empty, no icon will be shown.';
    img_p.className = 'floating_window_paragraph';

    const icon = document.createElement('input');
    icon.type = 'file';
    icon.name = 'icon';
    icon.accept = 'image/*';
    icon.className = 'edit-profile-input';
    icon.id = 'edit-profile-icon';

    const icon_preview = document.createElement('img');
    icon_preview.id = 'edit-profile-icon-preview';
    icon_preview.style.maxWidth = '500px';
    icon_preview.style.maxHeight = '500px';
    icon_preview.style.borderRadius = '10px';

    icon_preview.style.display = 'block';
    icon_preview.style.marginLeft = 'auto';
    icon_preview.style.marginRight = 'auto';

    if (curr_profile.profile_key) {
        icon_preview.src = '/download/' + curr_profile.profile_key;
    }

    icon.onchange = () => {
        play_click();

        if (icon.files && icon.files[0]) {
            const file = icon.files[0];
            const reader = new FileReader();
            reader.onload = (e) => {
                icon_preview.src = e.target.result;
                icon_preview.style.display = 'block';
            }
            reader.readAsDataURL(file);
        } else {
            icon_preview.style.display = 'none';
        }
    }

    const update_button = document.createElement('button');
    update_button.innerHTML = 'Update profile';
    update_button.className = 'edit-profile-button';
    update_button.onclick = () => {
        play_click();

        let json = {
            display_name: display_name.value,
            description: description.value,
        }

        update_profile(json, icon);

        hide_all_windows();
        view_profile(username);
    }

    win.appendChild(title);
    win.appendChild(display_name_h2);
    win.appendChild(display_name_p);
    win.appendChild(display_name);
    win.appendChild(description_h2);
    win.appendChild(description_p);
    win.appendChild(description);
    win.appendChild(img_h2);
    win.appendChild(img_p);
    win.appendChild(icon_preview);
    win.appendChild(icon);

    win.appendChild(document.createElement('br'));
    win.appendChild(document.createElement('br'));
    win.appendChild(update_button);

    document.body.appendChild(win);
}

async function view_profile(username) {
    set_path('/profile/' + username);
    play_click();

    const win = create_window('profile-window');

    if (username === get_cookie('username')) {
        const pen = document.createElement('img');

        pen.src = '/img/pen.svg';
        pen.className = 'edit-profile-watermark';
        pen.style.position = 'absolute';
        pen.style.top = '10px';
        pen.style.right = '50px';
        pen.style.width = '20px';
        pen.onclick = async () => {
            play_click();
            await edit_profile(username, await get_profile_for_user(username));
        }

        win.appendChild(pen);
    }

    let display_name = username;

    const profile = await get_profile_for_user(username);
    if (!profile) {
        const error = document.createElement('p');
        error.innerHTML = 'Profile not found!';
        error.className = 'error';
        win.appendChild(error);
        document.body.appendChild(win);
        return;
    }

    if (profile.display_name && profile.display_name !== username) {
        display_name = profile.display_name + " (" + username + ")";
    }

    if (profile.profile_key) {
        const img = document.createElement('img');
        img.src = '/download/' + (profile.profile_key || '');
        img.className = 'profile-watermark';
        img.id = 'profile-watermark';

        img.style.display = 'block';
        img.style.marginLeft = 'auto';
        img.style.marginRight = 'auto';
        img.style.maxWidth = '200px';
        img.style.maxHeight = '200px';
        img.style.borderRadius = '50%';

        win.appendChild(img);
    }

    const title = document.createElement('h1');
    title.innerHTML = 'Profile of ' + display_name;

    const description = document.createElement('p');
    description.innerHTML = profile.description || 'No description provided.';
    description.className = 'profile-description';
    description.id = 'profile-description';

    const view_uploads = document.createElement('button');
    view_uploads.innerHTML = 'View uploaded forwarders';
    view_uploads.className = 'view-uploads-button';
    view_uploads.onclick = () => {
        play_click();
        show_browse(username);
    }
    const view_sandbox = document.createElement('button');
    view_sandbox.innerHTML = 'View sandbox uploads';
    view_sandbox.className = 'view-sandbox-button';
    view_sandbox.style.marginLeft = '10px';
    view_sandbox.style.marginRight = '10px';
    view_sandbox.onclick = () => {
        play_click();
        show_sandbox(username);
    }

    // hardcoded swag for me
    if (username === "jacob") {
        const p = document.createElement('p');
        p.className = 'profile-swag';
        p.id = 'profile-swag';
        p.style.color = 'red';
        p.style.fontWeight = 'bold';
        p.style.backgroundColor = 'blue';
        p.style.fontFamily = 'Comic Sans MS';
        p.innerHTML = 'This account is owned and administered by the owner, maintainer and founder of Forwarder Factory. This paragraph is here as a sign of legitimacy and to prevent impersonation. GRAPHIC DESIGN IS MY PASSION!!!';
        p.onclick = () => {
            play_click();
            window.open('https://www.youtube.com/@ForwarderFactory', '_blank');
        }

        win.appendChild(p);
    }

    win.appendChild(title);
    win.appendChild(description);
    win.appendChild(view_uploads);
    win.appendChild(view_sandbox);

    document.body.appendChild(win);
}

function print_username(username, display_name, profile_key) {
    if (display_name == null) {
        return;
    }

    const span = document.createElement('span');
    span.className = 'logged-in-watermark';
    span.id = 'logged-in-watermark';
    span.style.position = 'absolute';
    span.style.top = '0';
    span.style.left = '0';
    span.style.padding = '10px';
    span.style.userSelect = 'none';
    span.style.cursor = 'pointer';

    if (is_phone()) {
        span.style.fontSize = '10px';
    }

    // Create either the profile image or fallback icon
    if (profile_key) {
        const img = document.createElement('img');

        img.src = "/download/" + profile_key;
        img.alt = display_name + "'s profile picture";
        img.style.width = '20px';
        img.style.height = '20px';

        if (is_phone()) {
            img.style.width = '10px';
            img.style.height = '10px';
        }
        img.style.borderRadius = '50%';
        img.style.marginRight = '5px';
        span.appendChild(img);
    } else {
        const i = document.createElement('i');
        i.className = 'fas fa-user';
        i.style.marginRight = '5px';
        span.appendChild(i);
    }

    const text = document.createTextNode(display_name);
    span.appendChild(text);

    span.onclick = () => {
        view_profile(username);
    }
    span.onmouseover = () => {
        span.style.transform = 'scale(1.1)';
    }
    span.onmouseleave = () => {
        span.style.transform = 'scale(1.0)';
    }

    document.body.appendChild(span);
}


function show_logout() {
    const logout = create_window('logout-window');
    const title = document.createElement('h1');
    title.innerHTML = 'Logout';
    title.className = 'floating_window_title';
    title.id = 'logout-window-title';

    const paragraph = document.createElement('p');
    paragraph.innerHTML = 'Are you sure you want to logout?';
    paragraph.className = 'floating_window_paragraph';
    paragraph.id = 'logout-window-paragraph';

    logout.appendChild(title);
    logout.appendChild(paragraph);

    const button = document.createElement('button');
    button.innerHTML = "Yes, I'm sure!";
    button.className = 'logout-button';
    button.onclick = () => {
        play_click();
        fetch('/api/try_logout', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
        })
            .then(response => {
                if (response.status === 204) {
                    hide_all_windows();
                    window.location.href = '/';
                } else {
                    throw new Error('Logout failed');
                }
            })
            .catch(error => {
                console.error('Error:', error);
                show_logout('Logout failed. Please try again.');
            });
    }

    const next_button = document.createElement('button');
    next_button.style.marginLeft = '10px';
    next_button.innerHTML = 'No, regret!';
    next_button.className = 'regret-button';
    next_button.onclick = () => {
        play_click();
        hide_all_windows();
    }

    logout.appendChild(button)
    logout.appendChild(next_button);
}

function show_sandbox_upload(_error = "") {
    play_click()
    set_path('/upload');
    hide_initial();

    let ret = {
        /*
        title,
        description,
        author,
        categories,
        file
         */
    };

    const assemble_request = () => {
        play_click();

        const url = '/api/try_upload_file';

        let json = { meta: {} };

        if (ret.title) json.meta.title = ret.title.value;
        if (ret.description) json.meta.description = ret.description.value;
        if (ret.author) json.meta.author = ret.author.value;
        if (ret.categories) json.meta.categories = ret.categories.value.split(',');

        const form = new FormData();

        form.append('json', new Blob([JSON.stringify(json)], { type: 'application/json' }));

        if (ret.files && ret.files.length > 0) {
            for (let i = 0; i < ret.files.length; i++) {
                form.append(ret.files[i].name, ret.files[i], ret.files[i].name);
            }
        }

        const loading = create_window('loading-window', { close_button: false, moveable: false, remove_existing: true, close_on_escape: false, close_on_click_outside: false });

        const title = document.createElement('h1');
        title.innerHTML = 'Uploading...';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please wait while we upload your file. This may take a few moments. Please do not close this window.';

        loading.appendChild(title);
        loading.appendChild(paragraph);

        fetch(url, {
            method: 'POST',
            body: form,
        })
            .then(response => {
                return response.json();
            })
            .then(json => {
                if (json.error_str) {
                    show_sandbox_upload(json.error_str);
                    return;
                }
                if (json.id) {
                    hide_all_windows();
                    show_file(json.id);
                    return;
                }
                if (json) {
                    throw new Error('Invalid response from server: ' + JSON.stringify(json));
                }
            })
            .catch((error) => {
                console.error('Error:', error);
            });
    }

    const finalize_details = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Confirm';

        const paragraph = document.createElement('p');
        paragraph.innerHTML += "Forwarder Factory reserves the right to ban users who upload harmful, malicious or otherwise dangerous files. ";
        paragraph.innerHTML += "Further, Forwarder Factory and its members, contributors and affiliates are not responsible for any damages caused by files uploaded to the site. ";
        paragraph.innerHTML += "We reserve the right to remove any forwarder at any time for any reason, such as but not limited to, a DMCA takedown request.";

        const submit = document.createElement('button');

        submit.innerHTML = 'Submit';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            assemble_request();
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.style.marginRight = '10px';
        back.onclick = () => {
            ask_for_file();
        }

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(document.createElement('br'));
        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    const ask_for_file = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'File';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please upload the files you are uploading.';  // Changed wording for plural
        paragraph.innerHTML += '<br>';

        const file = document.createElement('input');
        file.type = 'file';
        file.name = 'file';
        file.className = 'upload-input';
        file.id = 'upload-file';
        file.style.width = '80%';
        file.style.marginRight = '10px';
        file.multiple = true;  // Allow multiple files
        file.onclick = () => {
            play_click();
        }

        const submit = document.createElement('button');
        submit.innerHTML = 'Upload';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            ret.files = file.files;
            finalize_details();
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.style.marginRight = '10px';
        back.onclick = () => {
            ret.files = file.files;
            ask_for_categories();
        }

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(file);
        upload.appendChild(document.createElement('br'));
        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    const ask_for_categories = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Categories';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = "If you have any categories you'd like this file to fall under, please specify them (comma separated).";

        const categories = document.createElement('input');
        categories.type = 'text';
        categories.name = 'categories';
        categories.placeholder = 'Categories (comma separated)';
        categories.className = 'upload-input';
        categories.id = 'upload-categories';
        categories.style.width = '80%';
        categories.style.marginBottom = '10px';
        if (ret.categories !== undefined && ret.categories.value !== undefined) {
            categories.value = ret.categories.value;
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.style.marginRight = '10px';
        back.onclick = () => {
            ret.categories = categories;
            ask_for_author();
        }

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            ret.categories = categories;
            ask_for_file();
        }

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(categories);
        upload.appendChild(document.createElement('br'));
        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }
    const ask_for_author = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Author';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please provide the author/copyright holder of the file you are uploading. This will be displayed to users.';
        paragraph.innerHTML += "If you don't know, leave it blank. Do not put your own name here, unless you are the author of the file.";

        const author = document.createElement('input');
        author.type = 'text';
        author.name = 'author';
        author.placeholder = 'Author';
        author.className = 'upload-input';
        author.id = 'upload-author';
        author.style.width = '50%';
        author.style.marginBottom = '10px';
        author.onclick = () => {
            play_click();
        }

        if (ret.author !== undefined && ret.author.value !== undefined) {
            author.value = ret.author.value;
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.style.marginRight = '10px';
        back.onclick = () => {
            ret.author = author;
            ask_for_description();
        }

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            ret.author = author;
            ask_for_categories();
        }

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(author);
        upload.appendChild(document.createElement('br'));
        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    const ask_for_description = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Description';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please provide a description for your file. This will be displayed on the file page.';

        const input = document.createElement('textarea');
        input.name = 'description';
        input.placeholder = 'Description';
        input.className = 'upload-input';
        input.id = 'upload-description';
        input.style.width = '80%';
        input.style.height = '200px';
        if (ret.description !== undefined && ret.description.value !== undefined) {
            input.value = ret.description.value;
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.onclick = () => {
            ret.description = input.value;
            ask_for_title();
        }
        back.style.marginRight = '10px';

        const submit = document.createElement('button');

        submit.innerHTML = 'Continue';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            ret.description = input;
            ask_for_author();
        }

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(input);
        upload.appendChild(document.createElement('br'));
        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    const ask_for_title = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Title';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please provide a title to describe your file.';

        const title_input = document.createElement('input');
        title_input.type = 'text';
        title_input.name = 'title';
        title_input.placeholder = 'Title';
        title_input.className = 'upload-input';
        title_input.id = 'upload-title';
        title_input.style.width = '50%';
        title_input.style.marginBottom = '10px';
        title_input.onclick = () => {
            play_click();
        }
        if (ret.title !== undefined && ret.title.value !== undefined) {
            title_input.value = ret.title.value;
        }

        title_input.addEventListener('input', () => {
            title_input.value = title_input.value.substring(0, 30);
        });

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.onclick = () => {
            ret.title = title_input;
            show_upload();
        }
        back.style.marginRight = '10px';

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            ret.title = title_input;
            ask_for_description();
        }

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(title_input);

        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    ask_for_title();
}

function show_upload(_error = "") {
    play_click();
    set_path('/upload');

    hide_initial();

    let ret = {
        /*
        contentType,
        description,
        titleID,
        title,
        icon,
        banner,
        author,
        youtube,
        type,
        categories,
        location,
        vwiiCompatible,
        wad,
        */
    };

    const assemble_request = () => {
        play_click();

        const url = '/api/try_upload_forwarder';

        // username and password does not need to be specified, as the server will check the session
        // and get the username from there
        //
        // we need to pass:
        // - file fields (banner, icon, wad)
        // - other fields (in the form of a JSON object)
        //
        // content type however is irrelevant because if we're here, we're uploading a forwarder
        let json = { meta: {} };

        if (ret.description) json.meta.description = ret.description.value;
        if (ret.titleID) json.meta.title_id = ret.titleID.value;
        if (ret.title) json.meta.title = ret.title.value;
        if (ret.author) json.meta.author = ret.author.value;
        if (ret.youtube) json.meta.youtube = ret.youtube.value;
        if (ret.type) json.meta.type = ret.type.value;
        if (ret.location) json.meta.location = ret.location.value;
        if (ret.vwiiCompatible) json.meta.vwii_compatible = ret.vwiiCompatible.value;
        if (ret.categories) json.meta.categories = ret.categories.value.split(',');

        // create a form object
        const form = new FormData();

        form.append('json', new Blob([JSON.stringify(json)], { type: 'application/json' }));

        if (ret.banner && ret.banner.files) {
            form.append('banner', ret.banner.files[0]);
        } else {
            console.error('Banner file is missing or not selected');
        }

        if (ret.icon && ret.icon.files) {
            form.append('icon', ret.icon.files[0]);
        } else {
            console.error('Icon file is missing or not selected');
        }

        if (ret.wad && ret.wad.files) {
            form.append('wad', ret.wad.files[0]);
        } else {
            console.error('WAD file is missing or not selected');
        }

        if (ret.images && ret.images.files) {
            for (let i = 0; i < ret.images.files.length; i++) {
                form.append(ret.images.files[i].name, ret.images.files[i]);
            }
        }

        // loading screen
        const loading = create_window('loading-window', { close_button: false, moveable: false, remove_existing: true, close_on_escape: false, close_on_click_outside: false });

        const title = document.createElement('h1');
        title.innerHTML = 'Uploading...';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please wait while we upload your forwarder. This may take a few moments. Please do not close this window.';

        loading.appendChild(title);
        loading.appendChild(paragraph);

        fetch(url, {
            method: 'POST',
            body: form,
        })
            .then(response => {
                return response.json();
            })
            .then(json => {
                if (json.error_str) {
                    show_upload(json.error_str);
                    return;
                }
                if (json.id) {
                    show_forwarder(json.id);
                    return;
                }
                if (json) {
                    throw new Error('Invalid response from server: ' + JSON.stringify(json));
                }
            })
            .catch((error) => {
                console.error('Error:', error);
            });
    }
    const finalize_details = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Confirm';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = "By submitting this forwarder, you hereby pledge that you have made a good faith effort to ensure that the forwarder is not harmful to users' Wii consoles.";
        paragraph.innerHTML += " Forwarder Factory reserves the right to ban users who upload harmful, malicious or otherwise dangerous forwarders. ";
        paragraph.innerHTML += "Forwarder Factory and its members, contributors and affiliates are not responsible for any damages caused by forwarders uploaded to the site. ";
        paragraph.innerHTML += "Further, we reserve the right to remove any forwarder at any time for any reason, such as but not limited to, a DMCA takedown request.";

        const submit = document.createElement('button');

        submit.innerHTML = 'Submit';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            assemble_request();
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.style.marginRight = '10px';
        back.onclick = () => {
            ask_for_wad();
        }

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(document.createElement('br'));
        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    const ask_for_wad = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'WAD File';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please upload the WAD file of the forwarder you are uploading.';
        paragraph.innerHTML += '<br>';
        paragraph.innerHTML += 'This is the file that will be installed on the Wii.';
        paragraph.innerHTML += 'Please ensure the WAD file is not corrupt as best you can. Intentionally uploading a corrupt WAD file will result in a ban.';
        paragraph.innerHTML += '<br>';

        const wad = document.createElement('input');
        wad.type = 'file';
        wad.name = 'wad';
        wad.accept = '.wad';
        wad.className = 'upload-input';
        wad.id = 'upload-wad';
        wad.style.width = '80%';
        wad.style.marginRight = '10px';
        wad.onclick = () => {
            play_click();
        }

        // TODO: some kind of serverside banner brick testing?
        // TODO 2: banner previews? someone smarter than me can do that pretty please

        const submit = document.createElement('button');
        submit.innerHTML = 'Upload';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            ret.wad = wad;
            finalize_details();
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.style.marginRight = '10px';
        back.onclick = () => {
            ret.wad = wad;
            ask_for_type_location_categories();
        }

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(wad);
        upload.appendChild(document.createElement('br'));
        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    const ask_for_type_location_categories = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Type, Location, and Category';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = "We need the type of forwarder you're uploading, the location it forwards to, and the categories it belongs to.";
        paragraph.innerHTML += '<br>';
        paragraph.innerHTML += 'The type is the type of forwarder you are uploading. It can be a forwarder (i.e. it forwards to a program) or a channel (self contained)';
        paragraph.innerHTML += '<br>';
        paragraph.innerHTML += "The location is the location of the program the forwarder forwards to. This is only required for forwarders, and is not required for channels. Your uploaded forwarder will forward to this location on either the SD card or attached USB drive. Only one path should be specified.";

        const type = document.createElement('select');
        type.name = 'type';
        type.id = 'upload-type';
        type.className = 'upload-input';
        type.style.width = '80%';
        type.style.marginBottom = '10px';
        if (ret.type !== undefined && ret.type.value !== undefined) {
            type.value = ret.type.value;
        }

        const types = ['Select', 'Forwarder', 'Channel'];
        for (let i = 0; i < types.length; i++) {
            const option = document.createElement('option');
            option.value = types[i];
            option.innerHTML = types[i];
            type.appendChild(option);
        }

        const location = document.createElement('input');
        location.type = 'text';
        location.name = 'location';
        location.placeholder = 'Location (e.g. /apps/usbloader_gx/boot.dol)';
        location.className = 'upload-input';
        location.id = 'upload-location';
        location.style.width = '80%';
        location.style.marginBottom = '10px';
        location.style.display = 'none';
        if (ret.location !== undefined && ret.location.value !== undefined) {
            location.value = ret.location.value;
        }

        let unset = false;

        // if it's a forwarder, ask for location
        type.addEventListener('change', () => {
            const type = document.getElementById('upload-type');
            const selected_type = type.options[type.selectedIndex].value;
            const location = document.getElementById('upload-location');

            if (selected_type === 'Forwarder') {
                unset = false;
                location.style.display = '';
            } else if (selected_type === 'Channel') {
                unset = false;
                location.style.display = 'none';
            } else if (selected_type === 'Select') {
                unset = true;
                location.style.display = 'none';
            }
        });

        const categories = document.createElement('input');

        categories.type = 'text';
        categories.name = 'categories';
        categories.placeholder = 'Categories (comma separated)';
        categories.className = 'upload-input';
        categories.id = 'upload-categories';
        categories.style.width = '80%';
        categories.style.marginBottom = '10px';
        if (ret.categories !== undefined && ret.categories.value !== undefined) {
            categories.value = ret.categories.value;
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.style.marginRight = '10px';
        back.onclick = () => {
            ret.categories = categories;
            ret.location = location;
            ret.type = type;
            ask_for_youtube();
        }

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            ret.categories = categories;
            ret.location = location;
            ret.type = type;
            ask_for_wad();
        }

        // disable button if type is unset
        type.addEventListener('input', () => {
            const submit = document.getElementById('continue-upload-submit');
            submit.disabled = unset;
        });

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(type);
        upload.appendChild(location);
        upload.appendChild(categories);
        upload.appendChild(document.createElement('br'));
        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    const ask_for_youtube = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'YouTube Video (Optional)';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'If you have a YouTube video showcasing your forwarder, please provide the video URL here. This will be displayed on the forwarder page.';

        const youtube = document.createElement('input');
        youtube.type = 'text';
        youtube.name = 'youtube';
        youtube.placeholder = 'YouTube Video URL';
        youtube.className = 'upload-input';
        youtube.id = 'upload-youtube';
        youtube.style.width = '80%';
        youtube.style.marginRight = '10px';
        youtube.onclick = () => {
            play_click();
        }
        if (ret.youtube !== undefined && ret.youtube.value !== undefined) {
            youtube.value = ret.youtube.value;
        }

        // embed a preview of the video on input
        youtube.addEventListener('input', () => {
            const url = youtube.value;
            let video_id = url.split('v=')[1];
            const ampersand_position = video_id.indexOf('&');
            if (ampersand_position !== -1) {
                video_id = video_id.substring(0, ampersand_position);
            }

            // if missing youtube.com, remove the preview
            if (!url.includes('youtube.com')) {
                const preview = document.getElementById('upload-youtube-preview');
                if (preview) {
                    upload.removeChild(preview);
                }
            }

            const preview = document.getElementById('upload-youtube-preview');
            if (preview) {
                upload.removeChild(preview);
            }

            const iframe = document.createElement('iframe');
            iframe.src = `https://www.youtube.com/embed/${video_id}`;
            iframe.style.width = '100%';
            iframe.style.height = '200px';
            iframe.style.maxWidth = '50%';
            iframe.style.marginBottom = '10px';
            iframe.style.borderRadius = '5px';

            iframe.id = 'upload-youtube-preview';
            upload.appendChild(iframe);
            upload.appendChild(document.createElement('br'));

            const submit = document.getElementById('continue-upload-submit');
            const back = document.getElementById('back-upload-button');
            upload.removeChild(submit);
            upload.removeChild(back);
            upload.appendChild(back);
            upload.appendChild(submit);
        });

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.style.marginRight = '10px';
        back.onclick = () => {
            ret.youtube = youtube;
            ask_for_author();
        }

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            ret.youtube = youtube;
            ask_for_type_location_categories();
        }

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(youtube);
        upload.appendChild(document.createElement('br'));
        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    const ask_for_author = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Author';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please provide the author of the forwarder you are uploading. This will be displayed to users.';
        paragraph.innerHTML += "If you don't know, leave it blank. Do not put your own name here, unless you are the author of the forwarder.";

        const author = document.createElement('input');
        author.type = 'text';
        author.name = 'author';
        author.placeholder = 'Author';
        author.className = 'upload-input';
        author.id = 'upload-author';
        author.style.width = '50%';
        author.style.marginBottom = '10px';
        author.onclick = () => {
            play_click();
        }

        if (ret.author !== undefined && ret.author.value !== undefined) {
            author.value = ret.author.value;
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.style.marginRight = '10px';
        back.onclick = () => {
            ret.author = author;
            ask_for_banner();
        }

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            ret.author = author;
            ask_for_youtube();
        }

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(author);
        upload.appendChild(document.createElement('br'));
        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    const ask_for_images = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Upload Images';
        title.className = 'upload-window-title';
        title.id = 'upload-window-title';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'You can upload up to 10 images to showcase your forwarder. These will be displayed on the forwarder page.';
        paragraph.className = 'upload-window-paragraph';
        paragraph.id = 'upload-window-paragraph';
        paragraph.style.marginBottom = '10px';
        paragraph.style.marginTop = '10px';

        const image = document.createElement('input');
        image.type = 'file';
        image.accept = 'image/*';
        image.multiple = true;
        image.className = 'upload-input';
        image.id = 'upload-image';
        image.style.width = '80%';
        image.style.marginRight = '10px';

        const forbidden = ['icon', 'banner', 'wad', 'data', 'json'];
        let n;
        let index = 1;
        do {
            n = `image_upload_${index++}`;
        } while (forbidden.includes(n));
        image.name = n;

        image.onclick = () => {
            play_click();
        };

        const preview = document.createElement('div');
        preview.id = 'image-preview-container';
        preview.style.display = 'flex';
        preview.style.flexWrap = 'wrap';
        preview.style.marginTop = '10px';
        preview.style.gap = '10px';

        image.addEventListener('change', () => {
            preview.innerHTML = '';

            if (image.files.length > 10) {
                alert('You can upload a maximum of 10 images.');
                image.value = '';
                return;
            }

            Array.from(image.files).forEach(file => {
                const reader = new FileReader();
                reader.onload = function (e) {
                    const img = document.createElement('img');
                    img.src = e.target.result;
                    img.style.width = '100px';
                    img.style.height = '100px';
                    img.style.objectFit = 'cover';
                    img.style.borderRadius = '8px';
                    img.style.boxShadow = '0 2px 4px rgba(0,0,0,0.2)';
                    preview.appendChild(img);
                };
                reader.readAsDataURL(file);
            });
        });

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            ret.images = image;
            ask_for_author();
        }
        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.onclick = () => {
            ret.images = image;
            ask_for_banner();
        }
        back.style.marginRight = '10px';

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(image);
        upload.appendChild(preview);

        upload.appendChild(document.createElement('br'));
        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    };

    const ask_for_banner = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Upload a Banner';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please upload a banner. This can be a screenshot or animated GIF of what is displayed upon clicking on the icon on the Wii menu.<br>This will be displayed at the top of the forwarder page.';
        paragraph.innerHTML += '<br>';
        paragraph.innerHTML += 'The banner may be any image or video file. For the best chances of your forwarder being accepted, please ensure it does not contain significant parts of the Wii menu.';
        paragraph.innerHTML += '<br>';
        paragraph.innerHTML += 'Try and capture the essence of your forwarder in a single image.';

        const banner = document.createElement('input');
        banner.type = 'file';
        banner.name = 'banner';
        banner.accept = 'image/*,video/*';
        banner.className = 'upload-input';
        banner.id = 'upload-banner';
        banner.style.width = '80%';
        banner.style.marginRight = '10px';
        banner.onclick = () => {
            play_click();
        }

        // on select, add a preview of the image on the page
        banner.addEventListener('change', () => {
            const file = banner.files[0];
            const reader = new FileReader();

            reader.onload = (e) => {
                if (file.type.includes('video')) {
                    const video = document.createElement('video');
                    video.src = e.target.result;
                    video.style.width = '100%';
                    video.style.maxWidth = '300px';
                    video.style.height = 'auto';
                    video.style.marginBottom = '10px';
                    video.style.borderRadius = '5px';
                    video.controls = true;
                    video.playsInline = true;
                    video.type = 'video/webm';

                    const preview = document.getElementById('upload-banner-preview');
                    if (preview) {
                        upload.removeChild(preview);
                    }

                    upload.appendChild(video);
                    upload.appendChild(document.createElement('br'));
                    video.id = 'upload-banner-preview';
                } else {
                    const img = document.createElement('img');
                    img.src = e.target.result;
                    img.style.width = '100%';
                    img.style.maxWidth = '300px';
                    img.style.height = 'auto';
                    img.style.marginBottom = '10px';
                    img.style.borderRadius = '5px';

                    const preview = document.getElementById('upload-banner-preview');
                    if (preview) {
                        upload.removeChild(preview);
                    }

                    upload.appendChild(img);
                    upload.appendChild(document.createElement('br'));
                    img.id = 'upload-banner-preview';
                }

                const submit = document.getElementById('continue-upload-submit');
                const back = document.getElementById('back-upload-button');
                upload.removeChild(submit);
                upload.removeChild(back);
                upload.appendChild(back);
                upload.appendChild(submit);
            }

            reader.readAsDataURL(file);
        });

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            ret.banner = banner;
            ask_for_images();
        }
        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.onclick = () => {
            ret.banner = banner;
            ask_for_icon();
        }
        back.style.marginRight = '10px';

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(banner);
        upload.appendChild(document.createElement('br'));
        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    const ask_for_icon = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Upload an Icon';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please upload an icon for your forwarder. This will be displayed when people are browsing for forwarders.';
        paragraph.innerHTML += '<br>';
        paragraph.innerHTML += 'The icon may be any image file. For the best chances of your forwarder being accepted, please ensure it does not contain significant parts of the Wii menu.';
        paragraph.innerHTML += '<br>';
        paragraph.innerHTML += 'Try and capture the essence of your forwarder in a single image.';
        paragraph.innerHTML += '<br>';

        const icon = document.createElement('input');
        icon.type = 'file';
        icon.name = 'icon';
        icon.accept = 'image/*,video/*';
        icon.className = 'upload-input';
        icon.id = 'upload-icon';
        icon.style.width = '80%';
        icon.style.marginRight = '10px';
        icon.onclick = () => {
            play_click();
        }

        // on select, add a preview of the image on the page
        // client side so fuck validation, worst case i guess the client can fuck up his own computer
        icon.addEventListener('change', () => {
            const file = icon.files[0];
            const reader = new FileReader();

            reader.onload = (e) => {
                if (file.type.includes('video')) {
                    const video = document.createElement('video');
                    video.src = e.target.result;
                    video.style.width = '100%';
                    video.style.maxWidth = '300px';
                    video.style.height = 'auto';
                    video.style.marginBottom = '10px';
                    video.style.borderRadius = '5px';
                    video.controls = true;
                    video.playsInline = true;
                    video.type = 'video/webm';

                    const preview = document.getElementById('upload-icon-preview');
                    if (preview) {
                        upload.removeChild(preview);
                    }

                    upload.appendChild(video);
                    upload.appendChild(document.createElement('br'));
                    video.id = 'upload-icon-preview';
                } else {
                    const img = document.createElement('img');
                    img.src = e.target.result;
                    img.style.width = '100%';
                    img.style.maxWidth = '300px';
                    img.style.height = 'auto';
                    img.style.marginBottom = '10px';
                    img.style.borderRadius = '5px';

                    const preview = document.getElementById('upload-icon-preview');
                    if (preview) {
                        upload.removeChild(preview);
                    }

                    upload.appendChild(img);
                    upload.appendChild(document.createElement('br'));
                    img.id = 'upload-icon-preview';
                }

                const submit = document.getElementById('continue-upload-submit');
                const back = document.getElementById('back-upload-button');
                upload.removeChild(submit);
                upload.removeChild(back);
                upload.appendChild(back);
                upload.appendChild(submit);
            }

            reader.readAsDataURL(file);
        });

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            ret.icon = icon;
            ask_for_banner();
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.onclick = () => {
            ret.icon = icon;
            ask_for_description();
        }
        back.style.marginRight = '10px';

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(icon);
        upload.appendChild(document.createElement('br'));
        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    const ask_for_description = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Description';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please provide a description for your forwarder. This will be displayed on the forwarder page.';

        const input = document.createElement('textarea');
        input.name = 'description';
        input.placeholder = 'Description';
        input.className = 'upload-input';
        input.id = 'upload-description';
        input.style.width = '80%';
        input.style.height = '200px';
        if (ret.description !== undefined && ret.description.value !== undefined) {
            input.value = ret.description.value;
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.onclick = () => {
            ret.description = input.value;
            ask_for_title();
        }
        back.style.marginRight = '10px';

        const submit = document.createElement('button');

        submit.innerHTML = 'Continue';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            ret.description = input;
            ask_for_icon();
        }

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(input);
        upload.appendChild(document.createElement('br'));
        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    const ask_for_title = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Title (optional)';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = "If you want to specify a title for your forwarder, you can do so here. This is optional; if you don't specify a title, the forwarder's title will be retrieved automatically!";

        const title_input = document.createElement('input');
        title_input.type = 'text';
        title_input.name = 'title';
        title_input.placeholder = 'Title';
        title_input.className = 'upload-input';
        title_input.id = 'upload-title';
        title_input.style.width = '50%';
        title_input.style.marginBottom = '10px';
        title_input.onclick = () => {
            play_click();
        }
        if (ret.title !== undefined && ret.title.value !== undefined) {
            title_input.value = ret.title.value;
        }

        title_input.addEventListener('input', () => {
            title_input.value = title_input.value.substring(0, 30);
        });

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.onclick = () => {
            ret.title = title_input;
            ask_for_content_type();
        }
        back.style.marginRight = '10px';

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            ret.title = title_input;
            ask_for_description();
        }

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(title_input);

        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    const ask_for_content_type = () => {
        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Upload';
        title.className = 'floating_window_title';
        title.id = 'upload-window-title';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Have some interesting content to share? Upload it here! Please note that all uploads are subject to review.';
        paragraph.innerHTML += '<br>';
        paragraph.innerHTML += 'Please ensure your forwarder is not a duplicate of an existing forwarder. ';
        paragraph.innerHTML += 'Some fields will require knowledge of the forwarder you are uploading. You can obtain all of the information required ';
        paragraph.innerHTML += 'by using a tool such as ShowMiiWads or CustomizeMii. If you are unsure about anything, please leave it blank.';
        paragraph.innerHTML += '<br>';
        paragraph.innerHTML += "If you're uploading something else, please select 'Other'.";

        paragraph.className = 'floating_window_paragraph';
        paragraph.id = 'upload-window-paragraph';

        const content_type = document.createElement('select');
        content_type.name = 'content_type';
        content_type.id = 'content_type';
        content_type.className = 'upload-input';
        content_type.style.width = '100%';
        content_type.style.marginBottom = '10px';

        const content_types = ['Forwarder', 'Other'];
        for (let i = 0; i < content_types.length; i++) {
            const option = document.createElement('option');
            option.value = content_types[i];
            option.innerHTML = content_types[i];
            content_type.appendChild(option);
        }

        content_type.addEventListener('change', () => {
            // TODO: Maybe we can do something cool here?
            play_click();
        });

        const button = document.createElement('button');
        button.innerHTML = 'Continue';
        button.id = 'upload-continue-button';
        button.onclick = () => {
            // TODO: Handle future content types
            if (content_type.options[content_type.selectedIndex].value === 'Forwarder') {
                ask_for_title();
            } else {
                show_sandbox_upload();
            }
        }

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(content_type);
        upload.appendChild(button);

        if (_error) {
            const error = document.createElement('p');
            error.classList = 'error';
            error.innerHTML = _error;
            upload.appendChild(document.createElement('br'));
            upload.appendChild(document.createElement('br'));
            upload.appendChild(error);
        }
    }

    ask_for_content_type();
}

function post_comment_forwarder(id, comment_str) {
    const json = {
        forwarder_identifier: id,
        comment_text: comment_str
    };

    fetch('/api/comment_forwarder', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify(json),
    })
        .then(response => {
            if (response.status === 204) {
                show_forwarder(id);
            }
        })
        .catch((error) => {
            console.error('Error:', error);
        });
}

function post_comment_file(id, comment_str) {
    const json = {
        file_identifier: id,
        comment_text: comment_str
    };

    fetch('/api/comment_file', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify(json),
    })
        .then(response => {
            if (response.status === 204) {
                show_file(id);
            }
        })
        .catch((error) => {
            console.error('Error:', error);
        });
}

function delete_comment_file(id, comment_id) {
    const json = {
        file_identifier: id,
        comment_identifier: comment_id
    };

    fetch('/api/delete_comment_file', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify(json),
    })
        .then(response => {
            if (response.status === 204) {
                show_file(id);
            }
        })
        .catch((error) => {
            console.error('Error:', error);
        });
}

function delete_comment_forwarder(id, comment_id) {
    const json = {
        forwarder_identifier: id,
        comment_identifier: comment_id
    };

    fetch('/api/delete_comment_forwarder', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify(json),
    })
        .then(response => {
            if (response.status === 204) {
                show_forwarder(id);
            }
        })
        .catch((error) => {
            console.error('Error:', error);
        });
}

function update_stars_for_file(id, stars) {
    const json = {
        file_identifier: id,
        rating: stars
    };

    fetch('/api/rate_file', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify(json),
    })
        .then(response => {
            if (response.status === 204) {
                show_file(id);
            }
        })
        .catch((error) => {
            console.error('Error:', error);
        });
}

function delete_forwarder(id) {
    const json = {
        forwarder_identifier: id
    };

    fetch('/api/delete_forwarder', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify(json),
    })
        .then(response => {
            if (response.status === 204) {
                show_browse();
            }
        })
        .catch((error) => {
            console.error('Error:', error);
        });
}

function delete_file(id) {
    const json = {
        file_identifier: id
    };

    fetch('/api/delete_file', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify(json),
    })
        .then(response => {
            if (response.status === 204) {
                show_sandbox();
            }
        })
        .catch((error) => {
            console.error('Error:', error);
        });
}

function update_stars_for_forwarder(id, stars) {
    const json = {
        forwarder_identifier: id,
        rating: stars
    };

    fetch('/api/rate_forwarder', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify(json),
    })
        .then(response => {
            if (response.status === 204) {
                show_forwarder(id);
            }
        })
        .catch((error) => {
            console.error('Error:', error);
        });
}

let Forwarder = 0;
let Sandbox = 1;

async function draw_article(type, forwarder, id) {
    let _id = 'view-window';
    if (type === Sandbox) {
        _id = 'file-window';
    }

    const view_window = create_window( _id);
    view_window.style.display = 'block';
    view_window.style.overflowY = 'scroll';

    if (
        forwarder.meta.banner_type !== undefined &&
        forwarder.meta.banner_type !== null &&
        forwarder.banner_download_key &&
        forwarder.banner_thumbnail_download_key
    ) {
        const thumbnail = document.createElement('img');
        thumbnail.src = `/download/${forwarder.banner_thumbnail_download_key}`;
        thumbnail.className = 'view_floating_window_banner';
        thumbnail.id = 'view_floating_window_banner_thumbnail';
        thumbnail.style.cursor = 'pointer';

        document.title = forwarder.meta.title || 'Forwarder Details';
        document.title = forwarder?.meta?.title || 'No title provided';

        const meta_description = document.createElement('meta');
        meta_description.name = 'description';
        meta_description.content = forwarder?.meta?.description || 'No description provided';
        meta_description.id = 'view_floating_window_meta_description';

        const meta_keywords = document.createElement('meta');
        meta_keywords.name = 'keywords';
        meta_keywords.content = forwarder?.categories ? forwarder.categories.join(', ') : 'No categories provided';
        meta_keywords.id = 'view_floating_window_meta_keywords';

        const meta_author = document.createElement('meta');
        meta_author.name = 'author';
        meta_author.content = forwarder?.meta?.author || 'Unknown Author';
        meta_author.id = 'view_floating_window_meta_author';

        const og_title = document.createElement('meta');
        og_title.setAttribute('property', 'og:title');
        og_title.content = forwarder?.meta?.title || 'No title provided';
        og_title.id = 'view_floating_window_og_title';

        const og_description = document.createElement('meta');
        og_description.setAttribute('property', 'og:description');
        og_description.content = forwarder?.meta?.description || 'No description provided';
        og_description.id = 'view_floating_window_og_description';

        const og_image = document.createElement('meta');
        og_image.setAttribute('property', 'og:image');
        og_image.content = `/download/${forwarder.banner_thumbnail_download_key}`;
        og_image.id = 'view_floating_window_og_image';

        document.head.appendChild(meta_description);
        document.head.appendChild(meta_keywords);
        document.head.appendChild(meta_author);
        document.head.appendChild(og_title);
        document.head.appendChild(og_description);
        document.head.appendChild(og_image);

        let banner = null;
        let is_hover = false;

        thumbnail.addEventListener('mouseenter', () => {
            is_hover = true;

            const wrapper = document.createElement('div');
            wrapper.style.position = 'relative';

            const banner_type = forwarder.meta.banner_type;
            const is_video = banner_type === 'video';

            if (is_video) {
                banner = document.createElement('video');
                banner.src = `/download/${forwarder.banner_download_key}`;
                banner.controls = false;
                banner.autoplay = true;
                banner.loop = true;
                banner.muted = false;
                banner.playsInline = true;
                banner.type = 'video/webm';
            } else {
                banner = document.createElement('img');
                banner.src = `/download/${forwarder.banner_download_key}`;
            }

            banner.className = 'view_floating_window_banner scaling-in hidden';
            banner.id = 'view_floating_window_banner';
            banner.style.cursor = 'pointer';

            const spinner = document.createElement('div');
            spinner.className = 'loading-spinner';

            wrapper.appendChild(banner);
            wrapper.appendChild(spinner);
            view_window.replaceChild(wrapper, thumbnail);

            const show_banner = () => {
                spinner.classList.add('hidden');
                banner.classList.remove('hidden');
                requestAnimationFrame(() => {
                    banner.classList.remove('scaling-in');
                });
            };

            if (is_video) {
                banner.addEventListener('canplay', show_banner);
            } else {
                banner.onload = show_banner;
            }

            banner.addEventListener('mouseleave', () => {
                is_hover = false;
                banner.classList.add('scaling-out');
                setTimeout(() => {
                    if (!is_hover) {
                        view_window.replaceChild(thumbnail, wrapper);
                        banner = null;
                    }
                }, 300);
            });
        });

        view_window.appendChild(thumbnail);
    }

    const title = document.createElement('h1');
    if (forwarder.meta.title) {
        title.innerHTML = forwarder.meta.title;
    } else {
        title.innerHTML = "No title provided";
    }
    title.className = 'view_floating_window_title';
    view_window.appendChild(title);

    const uploader = document.createElement('p');
    uploader.className = 'view_floating_window_uploader';
    if (forwarder.uploader && forwarder.uploader !== '') {
        const profile = await get_profile_for_user(forwarder.uploader);
        if (profile.profile_key === '' || profile.profile_key === undefined) {
            const fa = document.createElement('i');
            fa.className = "fa-solid fa-circle-user";
            fa.style.marginRight = '5px';
            uploader.prepend(fa);
        } else {
            const img = document.createElement('img');
            img.src = `/download/${profile.profile_key}`;
            img.className = 'view_floating_window_uploader_icon';
            img.style.marginRight = '5px';
            img.style.maxWidth = '15px';
            img.style.maxHeight = '15px';
            img.style.borderRadius = '50%';
            uploader.prepend(img);
        }

        uploader.innerHTML += `Uploaded by <a href="/profile/${forwarder.uploader}">${profile.display_name}</a>`;

        view_window.appendChild(uploader);
    }

    const author = document.createElement('p');
    if (forwarder.meta.author) {
        author.innerHTML = `${forwarder.meta.author}`;
    } else {
        author.innerHTML = "No author provided";
    }
    author.className = 'view_floating_window_author';
    const fa = document.createElement('i');
    fa.className = 'fa-solid fa-user';
    fa.style.marginRight = '5px';
    author.prepend(fa);
    view_window.appendChild(author);

    const title_id = document.createElement('p');
    if (forwarder.meta.title_id) {
        title_id.innerHTML = `${forwarder.meta.title_id}`;
        const fa = document.createElement('i');
        fa.className = 'fa-solid fa-id-card';
        fa.style.marginRight = '5px';
        title_id.prepend(fa);
        view_window.appendChild(title_id);
    }

    const rating_container = document.createElement('div');
    rating_container.className = 'forwarder_floating_window_rating';

    const stars = [];
    let current_rating = Math.round(forwarder.average_rating) || 0;
    const rating_text = document.createElement('p');
    rating_text.style.marginTop = '5px';

    function update_stars(rating) {
        stars.forEach((star, index) => {
            if (index < rating) {
                star.style.filter = 'invert(49%) sepia(98%) saturate(497%) hue-rotate(3deg) brightness(102%) contrast(101%)';
                star.alt = 'Filled star';
            } else {
                star.style.filter = 'grayscale(100%) brightness(60%)';
                star.alt = 'Empty star';
            }
        });
    }

    for (let i = 1; i <= 5; i++) {
        const star = document.createElement('img');
        star.src = '/img/star.svg';
        star.style.marginRight = '5px';
        star.style.width = '20px';
        star.style.height = '20px';
        star.alt = 'Empty star';

        if (is_logged_in()) {
            star.addEventListener('click', () => {
                if (current_rating === i) {
                    current_rating = 0;
                } else {
                    current_rating = i;
                }
                update_stars(current_rating);
                if (type === Forwarder) {
                    update_stars_for_forwarder(id, current_rating);
                } else {
                    update_stars_for_file(id, current_rating);
                }
                rating_text.textContent = current_rating
                    ? `You rated: ${current_rating} star${current_rating > 1 ? 's' : ''}`
                    : 'Rating cleared';
            });

            star.addEventListener('mouseover', () => {
                update_stars(i);
            });
            star.addEventListener('mouseout', () => {
                update_stars(current_rating);
            });
        }

        stars.push(star);
        rating_container.appendChild(star);
    }

    if (forwarder.average_rating || forwarder.average_rating === 0) {
        if (forwarder.average_rating === 0) {
            rating_text.textContent = 'No ratings yet';
        } else {
            rating_text.textContent = `${forwarder.average_rating.toFixed(2)} stars (${forwarder.rating_count} ratings)`;
        }
    } else {
        rating_text.textContent = 'No rating data';
    }

    update_stars(current_rating);

    rating_container.appendChild(rating_text);
    view_window.appendChild(rating_container);

    const fw_type = document.createElement('p');
    fw_type.className = 'view_floating_window_type';
    if (forwarder.meta.type === 0) {
        fw_type.innerHTML = "Forwarder";
        const fa = document.createElement('i');
        fa.className = "fa-solid fa-arrow-right";
        fa.style.marginRight = '5px';
        fw_type.prepend(fa);
    } else if (forwarder.meta.type !== undefined && forwarder.meta.type !== null) {
        fw_type.innerHTML = "Channel";
        const fa = document.createElement('i');
        fa.className = "fa-solid fa-tv";
        fa.style.marginRight = '5px';
        fw_type.prepend(fa);
    }

    view_window.appendChild(fw_type);

    const location = document.createElement('p');
    location.className = 'view_floating_window_location';
    if (forwarder.meta.location && forwarder.meta.type === 0) {
        location.innerHTML = forwarder.meta.location;
        const fa = document.createElement('i');
        fa.className = "fa-solid fa-square-binary";
        fa.style.marginRight = '5px';
        location.prepend(fa);
        view_window.appendChild(location);
    }

    const categories = document.createElement('p');
    categories.className = 'view_floating_window_categories';
    if (forwarder.meta.categories && forwarder.meta.categories.length > 0) {
        categories.innerHTML = forwarder.meta.categories.join(', ');
        const fa = document.createElement('i');
        fa.className = "fa-solid fa-tag";
        fa.style.marginRight = '5px';
        categories.prepend(fa);
        view_window.appendChild(categories);
    }

    if (forwarder.meta.vwii_compatible !== undefined) {
        const vwii = document.createElement('p');
        vwii.className = 'view_floating_window_vwii';
        vwii.innerHTML = forwarder.meta.vwii_compatible ? 'Wii, vWii, Wii Mini' : 'Wii, Wii Mini';
        const fac = document.createElement('i');
        fac.className = "fa-solid fa-check";
        fac.style.marginRight = '5px';
        vwii.prepend(fac);
        view_window.appendChild(vwii);
    }

    const description = document.createElement('p');
    description.className = 'view_floating_window_description';
    if (forwarder.meta.description) {
        description.innerHTML = forwarder.meta.description;
        description.id = 'view_floating_window_description';
        view_window.appendChild(description);
    }

    if (forwarder.screenshots && forwarder.screenshots.length > 0) {
        const screenshots_h2 = document.createElement('h2');
        screenshots_h2.innerHTML = 'Screenshots';
        screenshots_h2.className = 'view_floating_window_screenshots_title';
        view_window.appendChild(screenshots_h2);

        const screenshots_container = document.createElement('div');
        screenshots_container.className = 'view_floating_window_screenshots';
        screenshots_container.style.display = 'flex';
        screenshots_container.style.flexWrap = 'wrap';
        screenshots_container.style.gap = '10px';
        screenshots_container.style.marginTop = '10px';
        screenshots_container.style.justifyContent = 'center';

        forwarder.screenshots.forEach((screenshot, index) => {
            const img = document.createElement('img');
            img.src = `/download/${screenshot}`;
            img.alt = `Screenshot ${index + 1}`;
            img.style.width = '100px';
            img.style.height = '100px';
            img.style.objectFit = 'cover';
            img.style.borderRadius = '8px';
            img.style.boxShadow = '0 2px 4px rgba(0,0,0,0.2)';

            // on click, show an enlarged version of the image
            img.onclick = () => {
                const _modal = document.getElementById('view_floating_window_modal');
                if (_modal) {
                    _modal.remove();
                }

                const enlarged_img = document.createElement('img');
                enlarged_img.src = img.src;
                enlarged_img.style.width = '80%';
                enlarged_img.style.maxWidth = '600px';
                enlarged_img.style.height = 'auto';
                enlarged_img.style.borderRadius = '8px';
                enlarged_img.style.boxShadow = '0 2px 4px rgba(0,0,0,0.2)';
                enlarged_img.style.zIndex = '99999';
                enlarged_img.style.position = 'fixed';
                enlarged_img.style.top = '50%';
                enlarged_img.style.left = '50%';
                enlarged_img.style.transform = 'translate(-50%, -50%)';

                const blacken = document.createElement('div');
                blacken.id = 'blacken';
                blacken.style.position = 'absolute';
                blacken.style.top = '0';
                blacken.style.left = '0';
                blacken.style.width = '100%';
                blacken.style.height = '100%';
                blacken.style.backgroundColor = 'black';
                blacken.style.opacity = '0.90';
                blacken.style.zIndex = '99999';

                const modal = document.createElement('div');
                modal.className = 'modal';
                modal.id = 'view_floating_window_modal';
                modal.onclick = () => {
                    modal.remove();
                    blacken.remove();
                };

                blacken.onclick = () => {
                    modal.remove();
                    blacken.remove();
                }

                modal.appendChild(enlarged_img);
                document.body.appendChild(blacken);
                document.body.appendChild(modal);
            }

            screenshots_container.appendChild(img);
        });

        view_window.appendChild(screenshots_container);
    }

    if (forwarder.meta.youtube) {
        const iframe = document.createElement('iframe');
        iframe.src = `https://www.youtube.com/embed/${forwarder.meta.youtube}`;
        iframe.style.width = '100%';
        iframe.style.height = '400px';
        iframe.style.maxWidth = '50%';
        iframe.style.marginBottom = '10px';
        iframe.style.borderRadius = '5px';
        view_window.appendChild(iframe);
    }

    const disclaimer = document.createElement('small');
    disclaimer.className = 'view_floating_window_disclaimer';
    disclaimer.innerHTML = "Forwarder Factory is providing this file as-is. Forwarder Factory and/or its contributors are not responsible for any damages caused by this file.";
    disclaimer.innerHTML += " By downloading this file, you agree to take full responsibility for any damages caused by this file. Precautions can and should be taken to prevent damage, such as in the case of forwarders and otherwise installable binaries using Priiloader or BootMii installed in boot2. We recommend testing all forwarders in an emulator before installing them on your console.";
    disclaimer.innerHTML += "<br/>";

    const download_h2 = document.createElement('h2');
    download_h2.innerHTML = 'Download';
    download_h2.className = 'view_floating_window_download_title';

    const download_p = document.createElement('p');
    download_p.innerHTML = 'Download the file(s) below for free. We recommend viewing the ratings and comments before downloading.';
    download_p.className = 'view_floating_window_download_text';

    view_window.appendChild(download_h2);
    view_window.appendChild(download_p);

    if (forwarder.data_download_key !== undefined && forwarder.data_download_key !== null) {
        const download_button = document.createElement('button');
        download_button.innerHTML = 'Download';
        download_button.className = 'view_floating_window_download';
        download_button.onclick = () => {
            window.location.href = "/download/" + forwarder.data_download_key;
        }

        view_window.appendChild(download_button);
    } else if (forwarder.data && forwarder.data.length > 0) {
        const grid = document.createElement('div');
        grid.className = 'grid';

        forwarder.data.forEach(file => {
            const file_div = document.createElement('div');
            file_div.className = 'file_div';
            file_div.id = `file_div_${file.download_key}`;

            const icon = document.createElement('i');
            icon.className = 'fas fa-file';
            file_div.appendChild(icon);

            const filename = document.createElement('p');
            filename.textContent = file.filename;
            filename.className = 'file_name';
            file_div.appendChild(filename);

            const download_button = document.createElement('button');
            download_button.textContent = 'Download';
            download_button.className = 'file_download_button';
            download_button.onclick = () => {
                window.location.href = `/download/${file.download_key}`;
            }
            file_div.appendChild(download_button);

            grid.appendChild(file_div);
        });

        view_window.appendChild(grid);
    }

    view_window.appendChild(document.createElement('br'));
    view_window.appendChild(document.createElement('br'));
    view_window.appendChild(disclaimer);

    if (forwarder.needs_review === true && get_cookie('user_type') === '1') {
        const accept_button = document.createElement('button');
        accept_button.innerHTML = 'Accept';
        accept_button.className = 'view_floating_window_accept';
        accept_button.style.marginRight = '10px';
        accept_button.onclick = () => {
            if (type === Forwarder) {
                accept_forwarder(id, true);
            } else {
                accept_file(id, true);
            }
        }

        const reject_button = document.createElement('button');
        reject_button.innerHTML = 'Reject';
        reject_button.className = 'view_floating_window_reject';
        reject_button.onclick = () => {
            if (type === Forwarder) {
                accept_forwarder(id, false);
            } else {
                accept_forwarder(id, false);
            }
        }

        view_window.appendChild(accept_button);
        view_window.appendChild(reject_button);
    }
    if (get_cookie('username') === forwarder.uploader || get_cookie('user_type') === '1') {
        const delete_button = document.createElement('button');
        delete_button.innerHTML = 'Delete';
        delete_button.className = 'view_floating_window_delete';
        delete_button.onclick = () => {
            play_click();

            const confirmation = create_window('confirmation-window');

            const confirmation_title = document.createElement('h1');
            confirmation_title.innerHTML = 'Are you sure?';
            confirmation_title.className = 'confirmation_window_title';
            confirmation_title.id = 'confirmation-window-title';

            const confirmation_text = document.createElement('p');
            confirmation_text.innerHTML = 'Are you sure you want to delete this? This action cannot be undone.';
            confirmation_text.className = 'confirmation_window_text';
            confirmation_text.id = 'confirmation-window-text';

            const yes_button = document.createElement('button');
            yes_button.innerHTML = 'Yes';
            yes_button.className = 'confirmation_window_yes';
            yes_button.id = 'confirmation-window-yes';
            yes_button.style.marginRight = '10px';
            yes_button.onclick = () => {
                play_click();

                confirmation.remove();

                if (type === Forwarder) {
                    delete_forwarder(id);
                } else {
                    delete_file(id);
                }
            }

            const no_button = document.createElement('button');
            no_button.innerHTML = 'No';
            no_button.className = 'confirmation_window_no';
            no_button.id = 'confirmation-window-no';
            no_button.onclick = () => {
                play_click();
                draw_article(type, forwarder, id);
            }

            confirmation.appendChild(confirmation_title);
            confirmation.appendChild(confirmation_text);
            confirmation.appendChild(yes_button);
            confirmation.appendChild(no_button);
        }

        view_window.appendChild(delete_button);
    }

    const post_comment = document.createElement('div');
    post_comment.className = 'view_floating_window_post_comment';

    const comment_h2 = document.createElement('h2');
    comment_h2.innerHTML = 'Post a Comment';
    comment_h2.className = 'view_floating_window_post_comment_title';
    post_comment.appendChild(comment_h2);

    const comment_p = document.createElement('p');
    if (is_logged_in()) {
        comment_p.innerHTML = 'Post a comment below. Your name will be displayed as the author of the comment.';
    } else {
        comment_p.innerHTML = 'You must be logged in to post a comment.';
    }
    comment_p.className = 'view_floating_window_post_comment_text';
    post_comment.appendChild(comment_p);

    if (is_logged_in()) {
        const comment_input = document.createElement('textarea');
        comment_input.className = 'view_floating_window_comment_input';
        comment_input.placeholder = 'Post a comment...';
        comment_input.id = 'view_floating_window_comment_input';
        comment_input.style.width = '50%';
        comment_input.style.height = '100px';
        comment_input.style.marginBottom = '10px';

        const post_button = document.createElement('button');
        post_button.innerHTML = 'Post Comment';
        post_button.className = 'view_floating_window_post_comment_button';
        post_button.onclick = () => {
            if (comment_input.value === '') {
                alert('Please enter a comment.');
                return;
            }

            play_click();
            if (type === Forwarder) {
                post_comment_forwarder(id, comment_input.value);
                hide_all_windows();
                show_forwarder(id);
            } else {
                post_comment_file(id, comment_input.value);
                hide_all_windows();
                show_file(id);
            }
        }

        post_comment.appendChild(comment_input);
        post_comment.appendChild(document.createElement('br'));
        post_comment.appendChild(post_button);
    }

    view_window.appendChild(post_comment);
    view_window.appendChild(document.createElement('br'));

    const reviews = forwarder.reviews || [];
    let index = 0;
    const bsize = 20;

    const comment_section = document.createElement('div');
    comment_section.className = 'view_floating_window_comment_section';

    view_window.appendChild(comment_section);

    async function draw_next() {
        const next = reviews.slice(index, index + bsize);
        index += bsize;

        for (const [comment_id, review] of next.entries()) {
            const profile = await get_profile_for_user(review.username);

            const comment_div = document.createElement('div');
            comment_div.className = 'view_floating_window_comment';
            comment_div.id = `view_floating_window_comment_${review.id}`;
            comment_div.style.marginBottom = '10px';

            const meta_div = document.createElement('div');
            meta_div.className = 'view_floating_window_comment_meta';

            const logo = document.createElement('img');
            if (profile.profile_key === '' || profile.profile_key === undefined) {
                logo.src = "/img/logo.svg";
            } else {
                logo.src = "/download/" + profile.profile_key;
            }
            logo.alt = `${review.username} logo`;
            logo.className = 'view_floating_window_comment_logo';
            logo.style.maxWidth = '30px';
            logo.style.maxHeight = '30px';
            logo.style.borderRadius = '50%';
            logo.style.marginRight = '10px';
            logo.style.paddingBottom = '-50px';

            const comment_author = document.createElement('span');
            comment_author.className = 'view_floating_window_comment_author';
            comment_author.innerHTML = `<strong>${review.username} </strong>`;

            if (comment_author === get_cookie("username") || get_cookie("user_type") === '1') {
                const delete_button = document.createElement('button');
                delete_button.className = 'view_floating_window_comment_delete';
                delete_button.innerHTML = '<i class="fa-solid fa-trash"></i>';
                delete_button.onclick = () => {
                    if (type === Forwarder) {
                        delete_comment_forwarder(id, comment_id);
                    } else {
                        delete_comment_file(id, comment_id);
                    }
                };
                comment_author.appendChild(delete_button);
            }

            const comment_date = document.createElement('span');
            comment_date.className = 'view_floating_window_comment_date';
            comment_date.textContent = new Date(review.timestamp).toLocaleString();

            const comment_text = document.createElement('p');
            comment_text.className = 'view_floating_window_comment_text';
            comment_text.innerHTML = review.comment;

            meta_div.appendChild(logo);
            meta_div.appendChild(comment_author);
            meta_div.appendChild(comment_date);

            comment_div.appendChild(meta_div);
            comment_div.appendChild(comment_text);

            comment_section.appendChild(comment_div);
        }
    }

    draw_next();
    view_window.addEventListener('scroll', () => {
        if (view_window.scrollTop + view_window.clientHeight >= view_window.scrollHeight) {
            draw_next();
        }
    });

    document.body.appendChild(view_window);
}

function accept_file(id, status) {
    const json = {
        files: {
            [id]: Boolean(status)
        },
    };

    fetch('/api/set_approval_for_uploads', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify(json),
    })
        .then(response => {
            if (response.status === 204) {
                show_file(id);
            }
        })
        .catch((error) => {
            console.error('Error:', error);
        });

    hide_all_windows();
}

function accept_forwarder(id, status) {
    const json = {
        forwarders: {
            [id]: Boolean(status)
        },
    };

    fetch('/api/set_approval_for_uploads', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify(json),
    })
        .then(response => {
            if (response.status === 204) {
                show_forwarder(id);
            }
        })
        .catch((error) => {
            console.error('Error:', error);
        });

    hide_all_windows();
}

async function show_forwarder(id) {
    set_path('/view/' + id);

    hide_initial();

    const get_forwarder = async (id) => {
        const filter_data = {
            filter: {
                accepted: false,
                identifier: id,
            }
        };

        const url = '/api/get_forwarders';
        const requestBody = JSON.stringify(filter_data);

        try {
            const response = await fetch(url, {
                method: 'POST',
                body: requestBody,
                headers: {
                    'Content-Type': 'application/json'
                }
            });

            const text = await response.text();

            if (text === '[]') {
                // todo: decent 404
                // for now just redir to /
                window.location.href = '/';
                return;
            }

            return JSON.parse(text);
        } catch (error) {
            console.error('Failed to get forwarder:', error);
            throw new Error('Failed to get forwarder. R u dum?');
        }
    };

    get_forwarder(id).then(_forwarder => {
        const forwarder = _forwarder.forwarders[0];
        if (!forwarder) {
            throw new Error('Forwarder not found');
        }

        draw_article(Forwarder, forwarder, id);
    }).catch(error => {
        console.error(error);
    });
}

function show_file(id) {
    set_path('/file/' + id);

    hide_initial();

    const get_file = async (id) => {
        const filter_data = {
            filter: {
                accepted: false,
                identifier: id,
            }
        };

        const url = '/api/get_files';
        const requestBody = JSON.stringify(filter_data);

        try {
            const response = await fetch(url, {
                method: 'POST',
                body: requestBody,
                headers: {
                    'Content-Type': 'application/json'
                }
            });

            const text = await response.text();

            if (text === '[]') {
                // todo: decent 404
                // for now just redir to /
                window.location.href = '/';
                return;
            }

            return JSON.parse(text);
        } catch (error) {
            console.error('Failed to get file:', error);
            throw new Error('Failed to get file. R u dum?');
        }
    };

    get_file(id).then(_file => {
        const file = _file.files[0];
        if (!file) {
            throw new Error('File not found');
        }

        draw_article(Sandbox, file, id);
    }).catch(error => {
        console.error(error);
    });
}

function show_sandbox(uploader = '') {
    set_path('/sandbox');
    hide_initial();

    const title = document.getElementById('page-header');
    if (title) {
        title.style.display = 'none';
    }

    const sandbox = create_window('sandbox-window');

    const _filter_data = {
        accepted: true,
        needs_review: false,
        filename: '',
        title: '',
        author: '',
        uploader: uploader,
        categories: [],
        submitted_before: undefined,
        submitted_after: undefined,
        search_string: ''
    };

    const update_previews = () => {
        const url = '/api/get_files';

        try {
            const requestBody = JSON.stringify({ filter: filter_data });

            fetch(url, {
                method: 'POST',
                body: requestBody,
                headers: {
                    'Content-Type': 'application/json'
                }
            })
                .then(response => response.text())
                .then(text => {
                    const json = JSON.parse(text);

                    const old = document.getElementById('sandbox-grid');
                    if (old) {
                        old.remove();
                    }

                    const container = document.createElement('div');
                    container.className = 'container';
                    container.id = 'sandbox-grid';

                    sandbox.appendChild(container);

                    let parent = document.createElement('div');

                    parent.className = 'grid';
                    parent.style.display = 'flex';
                    parent.style.flexDirection = 'row';
                    parent.style.flexWrap = 'wrap';
                    parent.style.gap = '10px';
                    parent.style.paddingTop = '10px';
                    parent.style.justifyContent = 'flex-start';

                    container.appendChild(parent);

                    let item_c = 0;

                    const files = json.files;
                    files.forEach(file => {
                        const meta = file.meta;
                        if (!meta) return;

                        if (item_c === 3) {
                            parent = document.createElement('div');
                            parent.className = 'grid';
                            parent.style.display = 'flex';
                            parent.style.flexDirection = 'row';
                            parent.style.flexWrap = 'wrap';
                            parent.style.gap = '10px';
                            parent.style.paddingTop = '10px';
                            parent.style.justifyContent = 'flex-start';
                            container.appendChild(parent);
                            item_c = 0;
                        }

                        const grid = document.createElement('div');
                        grid.className = 'window-item preview';
                        grid.id = file.id;
                        grid.style.padding = '10px';
                        grid.style.flex = '1 1 50px';
                        grid.style.boxSizing = 'border-box';

                        // get page_identifier
                        const page_identifier = file.page_identifier;
                        if (page_identifier) {
                            grid.onclick = () => {
                                play_click();
                                show_file(page_identifier);
                            }
                        }

                        const title = document.createElement('h2');
                        title.innerHTML = meta.title;
                        title.className = 'preview-title';

                        if (title.innerHTML.length > 20) {
                            title.innerHTML = title.innerHTML.substring(0, 20) + '...';
                        }

                        const author = document.createElement('p');
                        author.innerHTML = `${meta.author}`;
                        author.className = 'preview-author';

                        const icon = document.createElement('i');
                        icon.className = 'fa-solid fa-file';
                        icon.style.fontSize = '50px';
                        icon.style.marginBottom = '10px';

                        grid.appendChild(icon);
                        grid.appendChild(title);

                        if (meta.author && meta.author !== '') {
                            const fa = document.createElement('i');
                            fa.className = 'fa-solid fa-user';
                            fa.style.marginRight = '5px';
                            author.prepend(fa);
                            grid.appendChild(author);
                        }
                        parent.appendChild(grid);
                        item_c++;
                    });

                    if (files.size() === 0) {
                        const noResults = document.createElement('h2');
                        noResults.innerHTML = 'No results found. Why don\'t you find it for us?';
                        noResults.id = 'sandbox-no-results';
                        noResults.style.textAlign = 'center';
                        noResults.style.marginTop = '20px';
                        container.appendChild(noResults);
                    }

                    sandbox.appendChild(container);
                })
                .catch(() => {
                    console.error('Error fetching uploads');
                });
        } catch (error) {
            console.error('Error serializing filter_data:', error);
        }
    };

    const __filter_data = {
        set(target, property, value) {
            target[property] = value;
            update_previews();
            return true;
        }
    };

    const filter_data = new Proxy(_filter_data, __filter_data);

    const toggle_filter_div = () => {
        const filter_div = document.getElementById('sandbox-filter-div');
        if (!filter_div) {
            const div = document.createElement('div');
            div.id = 'sandbox-filter-div';
            div.style.position = 'absolute';
            div.style.top = '50%';
            div.style.left = '50%';
            div.style.transform = 'translate(-50%, -50%)';
            div.style.width = '50%';
            div.style.height = '50%';
            div.style.zIndex = '99999999';
            div.style.borderTopLeftRadius = '10px';
            div.style.borderTopRightRadius = '10px';
            div.style.borderBottomLeftRadius = '10px';
            div.style.borderBottomRightRadius = '10px';
            div.style.overflow = 'scroll';

            const close = document.createElement('a');
            close.innerHTML = '✕';
            close.id = 'sandbox-filter-close';
            close.style.position = 'fixed';
            close.style.top = '0';
            close.style.right = '0';
            close.style.padding = '10px';
            close.style.textDecoration = 'none';
            close.style.color = 'black';
            close.onclick = () => {
                play_click();
                toggle_filter_div();
            }

            const title_filter = document.createElement('input');
            title_filter.type = 'text';
            title_filter.name = 'title_filter';
            title_filter.placeholder = 'Title (exact match)';
            title_filter.className = 'sandbox-filter-input';
            title_filter.id = 'sandbox-title-filter';
            title_filter.style.marginBottom = '10px';
            if (filter_data.title) {
                title_filter.value = filter_data.title;
            }
            title_filter.style.marginTop = '10px';
            title_filter.style.marginRight = '10px';

            title_filter.addEventListener('input', () => {
                filter_data.title = title_filter.value;
            });

            const author_filter = document.createElement('input');
            author_filter.type = 'text';
            author_filter.name = 'author_filter';
            author_filter.placeholder = 'Author (exact match)';
            author_filter.className = 'sandbox-filter-input';
            author_filter.id = 'sandbox-author-filter';
            author_filter.style.marginBottom = '10px';
            author_filter.style.marginRight = '10px';
            if (filter_data.author) {
                author_filter.value = filter_data.author;
            }

            author_filter.addEventListener('input', () => {
                filter_data.author = author_filter.value;
            });

            const filename = document.createElement('input');
            filename.type = 'text';
            filename.name = 'filename';
            filename.placeholder = 'Filename (exact match)';
            filename.className = 'sandbox-filter-input';
            filename.id = 'sandbox-filename-filter';
            filename.style.marginBottom = '10px';
            filename.style.marginRight = '10px';
            if (filter_data.filename) {
                filename.value = filter_data.filename;
            }

            filename.addEventListener('input', () => {
                filter_data.filename = filename.value;
            });

            const uploader = document.createElement('input');
            uploader.type = 'text';
            uploader.name = 'uploader';
            uploader.placeholder = 'Uploader (exact match)';
            uploader.className = 'sandbox-filter-input';
            uploader.id = 'sandbox-uploader-filter';
            uploader.style.marginBottom = '10px';
            uploader.style.marginRight = '10px';
            if (filter_data.uploader) {
                uploader.value = filter_data.uploader;
            }

            const categories = document.createElement('input');
            categories.type = 'text';
            categories.name = 'categories';
            categories.placeholder = 'Categories (comma separated)';
            categories.className = 'sandbox-filter-input';
            categories.id = 'sandbox-categories-filter';
            categories.style.marginBottom = '10px';
            categories.style.marginRight = '10px';

            if (filter_data.categories) {
                categories.value = filter_data.categories.join(', ');
            }

            categories.addEventListener('input', () => {
                filter_data.categories = categories.value.split(',').map(item => item.trim());
            });

            const submitted_before = document.createElement('input');
            submitted_before.type = 'date';
            submitted_before.name = 'submitted_before';
            submitted_before.className = 'sandbox-filter-input';
            submitted_before.id = 'sandbox-submitted-before-filter';
            submitted_before.style.marginBottom = '10px';
            submitted_before.style.marginRight = '10px';
            if (filter_data.submitted_before !== undefined) {
                const date = new Date(filter_data.submitted_before);
                const year = date.getFullYear();
                const month = date.getMonth() + 1;
                const day = date.getDate();
                submitted_before.value = `${year}-${month}-${day}`;
            }
            submitted_before.addEventListener('input', () => {
                filter_data.submitted_before = new Date(submitted_before.value).getTime();
            });

            if (filter_data.type === undefined) {
                filter_data.type = -1;
            }

            const submitted_after = document.createElement('input');
            submitted_after.type = 'date';
            submitted_after.name = 'submitted_after';
            submitted_after.className = 'sandbox-filter-input';
            submitted_after.id = 'sandbox-submitted-after-filter';
            submitted_after.style.marginBottom = '10px';
            submitted_after.style.marginRight = '10px';
            if (filter_data.submitted_after !== undefined) {
                const date = new Date(filter_data.submitted_after);
                const year = date.getFullYear();
                const month = date.getMonth() + 1;
                const day = date.getDate();
                submitted_after.value = `${year}-${month}-${day}`;
            }

            submitted_after.addEventListener('input', () => {
                filter_data.submitted_after = new Date(submitted_after.value).getTime();
            });

            const create_label_for = (input, text) => {
                const label = document.createElement('label');
                label.innerHTML = text;
                label.htmlFor = input.id;
                label.style.marginRight = '10px';
                return label;
            }

            div.appendChild(close);

            div.appendChild(create_label_for(title_filter, 'Title'));
            div.appendChild(title_filter);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(author_filter, 'Author'));
            div.appendChild(author_filter);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(filename, 'Filename'));
            div.appendChild(filename);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(uploader, 'Uploader'));
            div.appendChild(uploader);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(categories, 'Categories'));
            div.appendChild(categories);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(submitted_before, 'Submitted Before'));
            div.appendChild(submitted_before);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(submitted_after, 'Submitted After'));
            div.appendChild(submitted_after);
            div.appendChild(document.createElement('br'));

            if (get_cookie('user_type') === "1") {
                const unverified = document.createElement('input');
                unverified.type = 'checkbox';
                unverified.name = 'unverified';
                unverified.id = 'sandbox-unverified-filter';

                unverified.addEventListener('input', () => {
                    filter_data.needs_review = unverified.checked;
                    filter_data.accepted = !unverified.checked;
                });

                const label = document.createElement('label');
                label.innerHTML = 'Unverified';
                label.htmlFor = 'sandbox-unverified-filter';
                label.style.marginLeft = '10px';
                label.style.marginRight = '10px';

                div.appendChild(label);
                div.appendChild(unverified);
            }

            sandbox.prepend(div);
        } else {
            sandbox.removeChild(filter_div);
        }
    }

    const filter_button = document.createElement('button');
    filter_button.innerHTML = 'Filter';
    filter_button.className = 'sandbox-button';
    filter_button.id = 'sandbox-filter-button';
    filter_button.onclick = () => {
        play_click();
        toggle_filter_div();
    }

    const search = document.createElement('input');
    search.type = 'text';
    search.name = 'search';
    search.placeholder = 'Search';
    search.className = 'sandbox-input';
    search.id = 'sandbox-search';
    search.style.width = '50%';
    search.style.marginBottom = '10px';
    search.style.marginTop = '10px';
    search.style.marginRight = '10px';

    search.style.position = 'absolute';
    search.style.top = '1';
    search.style.left = '50%';
    search.style.transform = 'translate(-50%, 0)';

    filter_button.style.position = 'absolute';
    filter_button.style.top = '1';
    filter_button.style.left = '20%';
    filter_button.style.marginBottom = '10px';
    filter_button.style.marginTop = '10px';
    filter_button.style.marginRight = '10px';

    search.addEventListener('input', () => {
        filter_data.search_string = search.value;
    });

    update_previews();

    document.body.prepend(search);
    document.body.prepend(filter_button);
}

function show_browse(uploader = '') {
    set_path('/browse');

    hide_initial();

    const title = document.getElementById('page-header');
    if (title) {
        title.style.display = 'none';
    }

    const browse = create_window('browse-window');

    const _filter_data = {
        accepted: true,
        needs_review: false,
        title: '',
        author: '',
        title_id_string: '',
        uploader: uploader,
        type: -1,
        categories: [],
        location: '',
        submitted_before: undefined,
        submitted_after: undefined,
        vwii: -1,
        search_string: ''
    };

    const update_previews = () => {
        const url = '/api/get_forwarders';

        try {
            const requestBody = JSON.stringify({ filter: filter_data });

            fetch(url, {
                method: 'POST',
                body: requestBody,
                headers: {
                    'Content-Type': 'application/json'
                }
            })
                .then(response => response.text())
                .then(text => {
                    const json = JSON.parse(text);

                    const old = document.getElementById('browse-grid');
                    if (old) {
                        old.remove();
                    }

                    const container = document.createElement('div');
                    container.className = 'container';
                    container.id = 'browse-grid';
                    browse.appendChild(container);

                    let parent = document.createElement('div');

                    parent.className = 'grid';
                    parent.style.display = 'flex';
                    parent.style.flexDirection = 'row';
                    parent.style.flexWrap = 'wrap';
                    parent.style.gap = '10px';
                    parent.style.paddingTop = '10px';
                    parent.style.justifyContent = 'flex-start';

                    container.appendChild(parent);

                    let item_c = 0;

                    const forwarders = json.forwarders;
                    forwarders.forEach(forwarder => {
                        const meta = forwarder.meta;
                        if (!meta) return;

                        if (item_c === 3) {
                            parent = document.createElement('div');
                            parent.className = 'grid';
                            parent.style.display = 'flex';
                            parent.style.flexDirection = 'row';
                            parent.style.flexWrap = 'wrap';
                            parent.style.gap = '10px';
                            parent.style.paddingTop = '10px';
                            parent.style.justifyContent = 'flex-start';
                            container.appendChild(parent);
                            item_c = 0;
                        }

                        const grid = document.createElement('div');
                        grid.className = 'window-item preview';
                        grid.id = forwarder.id;
                        grid.style.padding = '10px';
                        grid.style.flex = '1 1 50px';
                        grid.style.boxSizing = 'border-box';

                        // get page_identifier
                        const page_identifier = forwarder.page_identifier;
                        if (page_identifier) {
                            grid.onclick = () => {
                                play_click();
                                show_forwarder(page_identifier);
                            }
                        }

                        const title = document.createElement('h2');
                        title.innerHTML = meta.title;
                        title.className = 'preview-title';

                        if (title.innerHTML.length > 20) {
                            title.innerHTML = title.innerHTML.substring(0, 20) + '...';
                        }

                        const author = document.createElement('p');
                        author.innerHTML = `${meta.author}`;
                        author.className = 'preview-author';

                        const iconWrapper = document.createElement('div');
                        iconWrapper.style.position = 'relative';
                        iconWrapper.style.width = '100%';
                        iconWrapper.style.height = 'auto';

                        const thumbnail = document.createElement('img');
                        thumbnail.src = `/download/${forwarder.icon_thumbnail_download_key}`;
                        thumbnail.className = 'preview-icon';
                        thumbnail.style.cursor = 'pointer';

                        let icon = null;
                        let isHovering = false;

                        thumbnail.addEventListener('mouseenter', () => {
                            isHovering = true;

                            const iconType = forwarder.meta.icon_type;
                            const isVideo = iconType === 'video';

                            if (isVideo) {
                                icon = document.createElement('video');
                                icon.src = `/download/${forwarder.icon_download_key}`;
                                icon.type = 'video/webm';
                                icon.controls = false;
                                icon.autoplay = true;
                                icon.loop = true;
                                icon.muted = true;
                                icon.playsInline = true;
                                icon.type = 'video/webm';
                            } else {
                                icon = document.createElement('img');
                                icon.src = `/download/${forwarder.icon_download_key}`;
                            }

                            icon.className = 'preview-icon scaling-in hidden'; // hidden initially
                            icon.style.cursor = 'pointer';

                            const spinner = document.createElement('div');
                            spinner.className = 'loading-spinner';

                            iconWrapper.innerHTML = '';
                            iconWrapper.appendChild(icon);
                            iconWrapper.appendChild(spinner);

                            const showIcon = () => {
                                spinner.classList.add('hidden');
                                icon.classList.remove('hidden');
                                requestAnimationFrame(() => {
                                    icon.classList.remove('scaling-in');
                                });
                            };

                            if (isVideo) {
                                icon.addEventListener('canplay', showIcon);
                            } else {
                                icon.onload = showIcon;
                            }

                            icon.addEventListener('mouseleave', () => {
                                isHovering = false;
                                icon.classList.add('scaling-out');
                                setTimeout(() => {
                                    if (!isHovering) {
                                        iconWrapper.innerHTML = '';
                                        iconWrapper.appendChild(thumbnail);
                                        icon = null;
                                    }
                                }, 300);
                            });
                        });

                        iconWrapper.appendChild(thumbnail);

                        const type = document.createElement('p');
                        type.className = 'preview-type';

                        let is_location = false;
                        if (meta.type === 0) {
                            type.innerHTML = "Forwarder";
                            const fa = document.createElement('i');
                            fa.className = "fa-solid fa-arrow-right";
                            fa.style.marginRight = '5px';
                            type.prepend(fa);

                            type.onclick = () => {
                                if (!is_location) {
                                    type.innerHTML = `${meta.location}`;
                                    const fa = document.createElement('i');
                                    fa.className = "fa-solid fa-square-binary";
                                    fa.style.marginRight = '5px';
                                    type.prepend(fa);
                                } else {
                                    type.innerHTML = "Forwarder";
                                    const fa = document.createElement('i');
                                    fa.className = "fa-solid fa-arrow-right";
                                    fa.style.marginRight = '5px';
                                    type.prepend(fa);
                                }
                                is_location = !is_location;
                            }
                        } else {
                            type.innerHTML = "Channel";
                            const fa = document.createElement('i');
                            fa.className = "fa-solid fa-tv";
                            fa.style.marginRight = '5px';
                            type.prepend(fa);
                        }

                        grid.appendChild(iconWrapper);
                        grid.appendChild(title);

                        if (meta.author && meta.author !== '') {
                            const fa = document.createElement('i');
                            fa.className = 'fa-solid fa-user';
                            fa.style.marginRight = '5px';
                            author.prepend(fa);
                            grid.appendChild(author);
                        }
                        grid.appendChild(type);

                        parent.appendChild(grid);
                        item_c++;
                    });

                    // if no forwarders, display a message
                    if (forwarders.size() === 0) {
                        const noResults = document.createElement('h2');
                        noResults.innerHTML = 'No results found. Why don\'t you find it for us?';
                        noResults.id = 'browse-no-results';
                        noResults.style.textAlign = 'center';
                        noResults.style.marginTop = '20px';
                        container.appendChild(noResults);
                    }

                    // append to browse window
                    browse.appendChild(container);
                })
                .catch(() => {
                    console.error('Error fetching uploads');
                });
        } catch (error) {
            console.error('Error serializing filter_data:', error);
        }
    };

    const __filter_data = {
        set(target, property, value) {
            target[property] = value;
            update_previews();
            return true;
        }
    };

    const filter_data = new Proxy(_filter_data, __filter_data);

    const toggle_filter_div = () => {
        const filter_div = document.getElementById('browse-filter-div');
        if (!filter_div) {
            const div = document.createElement('div');
            div.id = 'browse-filter-div';
            div.style.position = 'absolute';
            div.style.top = '50%';
            div.style.left = '50%';
            div.style.transform = 'translate(-50%, -50%)';
            div.style.width = '50%';
            div.style.height = '50%';
            div.style.zIndex = '99999999';
            div.style.borderTopLeftRadius = '10px';
            div.style.borderTopRightRadius = '10px';
            div.style.borderBottomLeftRadius = '10px';
            div.style.borderBottomRightRadius = '10px';
            div.style.overflow = 'scroll';

            const close = document.createElement('a');
            close.innerHTML = '✕';
            close.id = 'browse-filter-close';
            close.style.position = 'fixed';
            close.style.top = '0';
            close.style.right = '0';
            close.style.padding = '10px';
            close.style.textDecoration = 'none';
            close.style.color = 'black';
            close.onclick = () => {
                play_click();
                toggle_filter_div();
            }

            const title_filter = document.createElement('input');
            title_filter.type = 'text';
            title_filter.name = 'title_filter';
            title_filter.placeholder = 'Title (exact match)';
            title_filter.className = 'browse-filter-input';
            title_filter.id = 'browse-title-filter';
            title_filter.style.marginBottom = '10px';
            if (filter_data.title) {
                title_filter.value = filter_data.title;
            }
            title_filter.style.marginTop = '10px';
            title_filter.style.marginRight = '10px';

            title_filter.addEventListener('input', () => {
                filter_data.title = title_filter.value;
            });

            const author_filter = document.createElement('input');
            author_filter.type = 'text';
            author_filter.name = 'author_filter';
            author_filter.placeholder = 'Author (exact match)';
            author_filter.className = 'browse-filter-input';
            author_filter.id = 'browse-author-filter';
            author_filter.style.marginBottom = '10px';
            author_filter.style.marginRight = '10px';
            if (filter_data.author) {
                author_filter.value = filter_data.author;
            }

            author_filter.addEventListener('input', () => {
                filter_data.author = author_filter.value;
            });

            const title_id_filter = document.createElement('input');
            title_id_filter.type = 'text';
            title_id_filter.name = 'title_id_filter';
            title_id_filter.placeholder = 'Title ID (exact match)';
            title_id_filter.className = 'browse-filter-input';
            title_id_filter.id = 'browse-title-id-filter';
            title_id_filter.style.marginBottom = '10px';
            title_id_filter.style.marginRight = '10px';
            if (filter_data.title_id_string) {
                title_id_filter.value = filter_data.title_id_string;
            }
            title_id_filter.addEventListener('input', () => {
                title_id_filter.value = title_id_filter.value.toUpperCase();
                title_id_filter.value = title_id_filter.value.replace(/[^A-Z0-9]/, '');
                title_id_filter.value = title_id_filter.value.substring(0, 4);
                filter_data.title_id_string = title_id_filter.value;
            });

            const uploader = document.createElement('input');
            uploader.type = 'text';
            uploader.name = 'uploader';
            uploader.placeholder = 'Uploader (exact match)';
            uploader.className = 'browse-filter-input';
            uploader.id = 'browse-uploader-filter';
            uploader.style.marginBottom = '10px';
            uploader.style.marginRight = '10px';
            if (filter_data.uploader) {
                uploader.value = filter_data.uploader;
            }

            const type = document.createElement('select');
            type.name = 'type';
            type.id = 'browse-type-filter';
            type.className = 'browse-filter-input';
            type.style.marginBottom = '10px';
            type.style.marginRight = '10px';

            const types = ['Select', 'Forwarder', 'Channel'];
            for (let i = 0; i < types.length; i++) {
                const option = document.createElement('option');
                option.value = types[i];
                option.innerHTML = types[i];
                type.appendChild(option);
            }

            type.addEventListener('input', () => {
                if (type.options[type.selectedIndex].value === 'Select') {
                    filter_data.type = -1;
                }
                if (type.options[type.selectedIndex].value === 'Forwarder') {
                    filter_data.type = 0;
                }
                if (type.options[type.selectedIndex].value === 'Channel') {
                    filter_data.type = 1;
                }
            });

            if (filter_data.type !== undefined) {
                if (filter_data.type === -1) {
                    type.selectedIndex = 0;
                } else if (filter_data.type === 0) {
                    type.selectedIndex = 1;
                } else if (filter_data.type === 1) {
                    type.selectedIndex = 2;
                }
            }

            const categories = document.createElement('input');
            categories.type = 'text';
            categories.name = 'categories';
            categories.placeholder = 'Categories (comma separated)';
            categories.className = 'browse-filter-input';
            categories.id = 'browse-categories-filter';
            categories.style.marginBottom = '10px';
            categories.style.marginRight = '10px';

            if (filter_data.categories) {
                categories.value = filter_data.categories.join(', ');
            }

            categories.addEventListener('input', () => {
                filter_data.categories = categories.value.split(',').map(item => item.trim());
            });

            const location = document.createElement('input');
            location.type = 'text';
            location.name = 'location';
            location.placeholder = 'Location (exact match)';
            location.className = 'browse-filter-input';
            location.id = 'browse-location-filter';
            location.style.marginBottom = '10px';
            location.style.marginRight = '10px';
            if (filter_data.location) {
                location.value = filter_data.location;
            }

            const submitted_before = document.createElement('input');
            submitted_before.type = 'date';
            submitted_before.name = 'submitted_before';
            submitted_before.className = 'browse-filter-input';
            submitted_before.id = 'browse-submitted-before-filter';
            submitted_before.style.marginBottom = '10px';
            submitted_before.style.marginRight = '10px';
            if (filter_data.submitted_before !== undefined) {
                const date = new Date(filter_data.submitted_before);
                const year = date.getFullYear();
                const month = date.getMonth() + 1;
                const day = date.getDate();
                submitted_before.value = `${year}-${month}-${day}`;
            }
            submitted_before.addEventListener('input', () => {
                filter_data.submitted_before = new Date(submitted_before.value).getTime();
            });

            if (filter_data.type === undefined) {
                filter_data.type = -1;
            }

            const submitted_after = document.createElement('input');
            submitted_after.type = 'date';
            submitted_after.name = 'submitted_after';
            submitted_after.className = 'browse-filter-input';
            submitted_after.id = 'browse-submitted-after-filter';
            submitted_after.style.marginBottom = '10px';
            submitted_after.style.marginRight = '10px';
            if (filter_data.submitted_after !== undefined) {
                const date = new Date(filter_data.submitted_after);
                const year = date.getFullYear();
                const month = date.getMonth() + 1;
                const day = date.getDate();
                submitted_after.value = `${year}-${month}-${day}`;
            }

            submitted_after.addEventListener('input', () => {
                filter_data.submitted_after = new Date(submitted_after.value).getTime();
            });

            if (filter_data.type === undefined) {
                filter_data.type = -1;
            }

            const vwii = document.createElement('select');
            vwii.name = 'vwii';
            vwii.id = 'browse-vwii-filter';
            vwii.className = 'browse-filter-input';
            vwii.style.marginBottom = '10px';
            vwii.style.marginRight = '10px';

            const vwii_options = ['Select', 'Yes', 'No'];
            for (let i = 0; i < vwii_options.length; i++) {
                const option = document.createElement('option');
                option.value = vwii_options[i];
                option.innerHTML = vwii_options[i];
                vwii.appendChild(option);
            }

            if (filter_data.vwii !== undefined) {
                if (filter_data.vwii <= 0) {
                    vwii.selectedIndex = 0;
                } else if (filter_data.vwii === 1) {
                    vwii.selectedIndex = 1;
                } else if (filter_data.vwii === 2) {
                    vwii.selectedIndex = 2;
                }
            }

            vwii.addEventListener('input', () => {
                if (vwii.options[vwii.selectedIndex].value === 'Select') {
                    filter_data.vwii = -1;
                }
                if (vwii.options[vwii.selectedIndex].value === 'Yes') {
                    filter_data.vwii = 1;
                }
                if (vwii.options[vwii.selectedIndex].value === 'No') {
                    filter_data.vwii = 0;
                }
            });

            if (filter_data.vwii === undefined) {
                filter_data.vwii = -1;
            }

            const create_label_for = (input, text) => {
                const label = document.createElement('label');
                label.innerHTML = text;
                label.htmlFor = input.id;
                label.style.marginRight = '10px';
                return label;
            }

            div.appendChild(close);

            div.appendChild(create_label_for(title_filter, 'Title'));
            div.appendChild(title_filter);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(author_filter, 'Author'));
            div.appendChild(author_filter);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(uploader, 'Uploader'));
            div.appendChild(uploader);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(title_id_filter, 'Title ID'));
            div.appendChild(title_id_filter);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(type, 'Type'));
            div.appendChild(type);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(categories, 'Categories'));
            div.appendChild(categories);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(location, 'Location'));
            div.appendChild(location);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(submitted_before, 'Submitted Before'));
            div.appendChild(submitted_before);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(submitted_after, 'Submitted After'));
            div.appendChild(submitted_after);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(vwii, 'vWii support'));
            div.appendChild(vwii);
            div.appendChild(document.createElement('br'));

            if (get_cookie('user_type') === "1") {
                const unverified = document.createElement('input');
                unverified.type = 'checkbox';
                unverified.name = 'unverified';
                unverified.id = 'browse-unverified-filter';

                unverified.addEventListener('input', () => {
                    filter_data.needs_review = unverified.checked;
                    filter_data.accepted = !unverified.checked;
                });

                const label = document.createElement('label');
                label.innerHTML = 'Unverified';
                label.htmlFor = 'browse-unverified-filter';
                label.style.marginLeft = '10px';
                label.style.marginRight = '10px';

                div.appendChild(label);
                div.appendChild(unverified);
            }

            browse.prepend(div);
        } else {
            browse.removeChild(filter_div);
        }
    }

    const filter_button = document.createElement('button');
    filter_button.innerHTML = 'Filter';
    filter_button.className = 'browse-button';
    filter_button.id = 'browse-filter-button';
    filter_button.onclick = () => {
        play_click();
        toggle_filter_div();
    }

    const search = document.createElement('input');
    search.type = 'text';
    search.name = 'search';
    search.placeholder = 'Search';
    search.className = 'browse-input';
    search.id = 'browse-search';
    search.style.width = '50%';
    search.style.marginBottom = '10px';
    search.style.marginTop = '10px';
    search.style.marginRight = '10px';

    search.style.position = 'absolute';
    search.style.top = '1';
    search.style.left = '50%';
    search.style.transform = 'translate(-50%, 0)';

    filter_button.style.position = 'absolute';
    filter_button.style.top = '1';
    filter_button.style.left = '20%';
    filter_button.style.marginBottom = '10px';
    filter_button.style.marginTop = '10px';
    filter_button.style.marginRight = '10px';

    search.addEventListener('input', () => {
        filter_data.search_string = search.value;
    });

    update_previews();

    document.body.prepend(search);
    document.body.prepend(filter_button);
}

function show_admin() {
    set_path('/admin');
    hide_initial();

    const admin = create_window('admin-window');

    const title = document.createElement('h1');
    title.innerHTML = 'Admin Panel';
    title.className = 'admin-title';
    admin.appendChild(title);

    const unfinished = document.createElement('p');
    unfinished.innerHTML = 'This page is unfinished and is not yet functional.';
    unfinished.innerHTML += ' In the meantime, please use SQLite on the server to manage users.';
    unfinished.className = 'admin-unfinished';
    admin.appendChild(unfinished);
}

function generate_stars(n, w) {
    const size = 4;
    const win_box = w.getBoundingClientRect();

    for (let i = 0; i < n; i++) {
        const star = document.createElement('div');

        star.className = 'star';
        star.style.position = 'absolute';
        star.style.width = `${size}px`;
        star.style.height = `${size}px`;
        star.style.backgroundColor = 'white';
        star.style.borderRadius = '50%';

        const max_top = w.clientHeight - size;
        const max_left = w.clientWidth - size;

        star.style.top = `${Math.random() * max_top}px`;
        star.style.left = `${Math.random() * max_left}px`;

        star.style.setProperty('--random-x', Math.random() - 0.5);
        star.style.setProperty('--random-y', Math.random() - 0.5);

        star.style.animationDuration = `${Math.random() * 10 + 10}s`;

        w.appendChild(star);
    }
}

function generate_sprites(container, url, img_properties = {}, size = 24, distance = 48) {
    const wrapper = document.createElement('div');
    const random_string = Math.random().toString(36).substring(2, 15);
    wrapper.id = 'tiled-wrapper-' + random_string;

    Object.assign(wrapper.style, {
        position: 'absolute',
        top: `-${distance}px`,
        left: `-${distance}px`,
        width: `calc(100% + ${distance * 2}px)`,
        height: `calc(100% + ${distance * 2}px)`,
        pointerEvents: 'none',
        overflow: 'hidden',
        willChange: "transform",
        zIndex: '-1'
    });

    const animate = (el) => {
        let start = null;

        const step = (timestamp) => {
            if (!start) start = timestamp;

            const progress = (timestamp - start) % 4000;
            const fraction = progress / 4000;

            const x = -distance * fraction;
            const y = distance * fraction;

            el.style.transform = `translate(${x}px, ${y}px)`;
            requestAnimationFrame(step);
        }

        requestAnimationFrame(step);
    }

    animate(wrapper);

    const cols = Math.ceil((container.offsetWidth + distance * 2) / distance);
    const rows = Math.ceil((container.offsetHeight + distance * 2) / distance);

    for (let y = 0; y < rows; y++) {
        for (let x = 0; x < cols; x++) {
            if ((x + y) % 2 === 0) {
                const sprite = document.createElement('div');
                sprite.className = 'sprite';
                sprite.style.left = `${x * distance}px`;
                sprite.style.top = `${y * distance}px`;
                sprite.style.position = "absolute";
                sprite.style.width = `${size}px`;
                sprite.style.height = `${size}px`;
                sprite.style.backgroundImage = `url(${url})`;
                sprite.style.backgroundSize = "cover";

                const filters = [];

                if (img_properties.invert) filters.push('invert(1)');
                if (img_properties.random_colors) filters.push(`hue-rotate(${Math.random() * 360}deg)`);
                if (img_properties.hue !== undefined) filters.push(`hue-rotate(${img_properties.hue}deg)`);
                if (img_properties.monochrome) filters.push('grayscale(1)');

                if (filters.length > 0) sprite.style.filter = filters.join(' ');

                sprite.style.opacity = img_properties.opacity !== undefined ? img_properties.opacity : 0.25;

                wrapper.appendChild(sprite);
            }
        }
    }

    container.prepend(wrapper);
}


function get_posts(topic_id, start_index = 0, end_index = -1) {
    if (topic_id == null || topic_id === '') {
        return Promise.resolve([]);
    }

    const url = '/api/get_posts';
    const requestBody = JSON.stringify({ topic_id: topic_id, start_index: start_index, end_index: end_index });
    return fetch(url, {
        method: 'POST',
        body: requestBody,
        headers: {
            'Content-Type': 'application/json'
        }
    })
    .then (response => response.text())
    .then (text => {
        const json = JSON.parse(text);
        if (json.error) {
            console.error(json.error);
            return Promise.resolve([]);
        }

        return json.posts;
    })
    .catch(error => {
        console.error('Error fetching posts:', error);
        return Promise.resolve([]);
    });
}

function get_topics(start_index = 0, end_index = -1) {
    // fetch /api/get_topics
    const url = '/api/get_topics';
    const requestBody = JSON.stringify({ start_index: start_index, end_index: end_index });
    return fetch(url, {
        method: 'POST',
        body: requestBody,
        headers: {
            'Content-Type': 'application/json'
        }
    })
    .then(response => response.text())
    .then(text => {
        const json = JSON.parse(text);
        if (json.error) {
            console.error(json.error);
            return [];
        }

        return json.topics;
    })
    .catch(error => {
        console.error('Error fetching topics:', error);
        return [];
    });
}

function show_post(post_id, topic_id = '') {
    hide_all_windows();
    set_path('/post');

    if (post_id === null || post_id === '') {
        console.error('post_id is null');
        return;
    }

    // get the post from the server
    const url = '/api/get_posts';
    const requestBody = JSON.stringify({ post_id: post_id, topic_id: topic_id });
    fetch(url, {
        method: 'POST',
        body: requestBody,
        headers: {
            'Content-Type': 'application/json'
        }
    })
    .then(response => response.text())
    .then(text => {
        const json = JSON.parse(text);
        if (json.error) {
            console.error(json.error);
            return;
        }

        const posts = json.posts;
        let post;
        posts.forEach(p => {
            if (p.identifier === post_id) {
                post = p;
            }
        });

        if (post === undefined) {
            console.error(`Post with ID ${post_id} not found`);
            return;
        }

        set_path('/post/' + post_id);

        const post_window = create_window('post-window-' + (topic_id || "root") + ("-" + post_id || "root"), { classes: ["forum_window"], close_button: true, back_button: null, close_on_escape: true, function_on_close: () => {
            hide_all_windows();
            show_topic(topic_id || post.topic_id);
        }
        });

        const title = document.createElement('h1');
        title.innerHTML = post.title || 'No title';
        title.className = 'post-title';

        const author = document.createElement('p');
        author.innerHTML = `Posted by ${post.created_by} on ${new Date(post.created_at).toLocaleDateString()}`;
        author.className = 'post-author';

        const content = document.createElement('div');
        content.innerHTML = post.text || 'No content';
        content.className = 'post-content';

        post_window.appendChild(title);
        post_window.appendChild(author);
        post_window.appendChild(content);

        // in a grid, show files. they're in data/x/download_key. We can display data/x/filename
        if (post.data && post.data.length > 0) {
            const files_grid = document.createElement('div');
            files_grid.className = 'post-files-grid';

            post.data.forEach(file => {
                const file_div = document.createElement('div');
                file_div.className = 'post-file';
                file_div.id = file.download_key;

                const file_name = document.createElement('p');
                file_name.innerHTML = file.filename || 'No filename';
                file_name.className = 'post-file-name';
                file_name.onclick = () => {
                    play_click();
                    window.location.href = `/download/${file.download_key}`;
                }
                file_name.style.color = 'blue';

                file_div.appendChild(file_name);
                files_grid.appendChild(file_div);
            });

            post_window.appendChild(files_grid);
        }

        const reply_h2 = document.createElement('h2');
        reply_h2.innerHTML = 'Reply to this post';
        reply_h2.className = 'post-reply-button';

        const reply_textarea = document.createElement('textarea');
        reply_textarea.className = 'post-reply-textarea';
        reply_textarea.placeholder = 'Write your reply here...';
        reply_textarea.rows = 5;
        reply_textarea.cols = 50;

        const file_uploads = document.createElement('input');
        file_uploads.type = 'file';
        file_uploads.multiple = true;
        file_uploads.className = 'post-file-upload';
        file_uploads.accept = '*/*';
        file_uploads.style.marginRight = '10px';

        const reply_button = document.createElement('button');
        reply_button.innerHTML = 'Reply';
        reply_button.className = 'post-reply-button';
        reply_button.onclick = () => {
            play_click();

            // 1. must be sent to /api/comment_post
            // 2. must be as a multipart, with the json being name="json"
            // 3. the json must contain the post_id, topic_id, text
            const formData = new FormData();
            formData.append('json', JSON.stringify({
                "post_id": post_id,
                "topic_id": topic_id,
                "comment": reply_textarea.value,
            }));

            if (file_uploads.files.length > 0) {
                for (let i = 0; i < file_uploads.files.length; i++) {
                    formData.append('file-' + i, file_uploads.files[i]);
                }
            }

            fetch('/api/comment_post', {
                method: 'POST',
                body: formData,
                headers: {
                    'Accept': 'application/json'
                }
            })
            .then(response => response.text())
            .then(text => {
                show_post(post_id, topic_id);
            });
        }

        // must not be closed, or admin, to reply
        if (get_cookie('user_type') === "1" || post.open) {
            post_window.appendChild(reply_h2);
            post_window.appendChild(reply_textarea);
            post_window.appendChild(document.createElement('br'));
            post_window.appendChild(file_uploads);
            post_window.appendChild(reply_button);
        }

        const replies_h2 = document.createElement('h2');
        replies_h2.innerHTML = 'Replies';
        replies_h2.className = 'post-replies-title';

        post_window.appendChild(replies_h2);

        // now print all comments for this post
        // they're in the comments array
        if (post.comments && post.comments.length > 0) {
            const search_input = document.createElement('input');

            search_input.type = 'text';
            search_input.id = 'comment_search_input';
            search_input.placeholder = 'Search comments...';
            search_input.className = 'post-comment-search';
            search_input.style.margin = '5px';

            post_window.appendChild(search_input);

            search_input.addEventListener('input', (event) => {
                const raw_query = event.target.value.trim().toLowerCase();
                search_query = raw_query;

                if (!raw_query) {
                    filtered_comments = [];
                    load_comments(1);
                    return;
                }

                const filters = {
                    author: null,
                    content: null,
                    file: null,
                    date: null,
                    any: []
                };

                raw_query.split(/\s+/).forEach(term => {
                    if (term.startsWith('author:')) {
                        filters.author = term.slice(7);
                    } else if (term.startsWith('content:')) {
                        filters.content = term.slice(8);
                    } else if (term.startsWith('file:')) {
                        filters.file = term.slice(5);
                    } else if (term.startsWith('date:')) {
                        filters.date = term.slice(5);
                    } else {
                        filters.any.push(term);
                    }
                });

                filtered_comments = post.comments.filter(comment => {
                    const comment_text = (comment.comment || '').toLowerCase();
                    const author = (comment.created_by || '').toLowerCase();
                    const date_str = new Date(comment.created_at).toLocaleDateString().toLowerCase();
                    const file_names = (comment.data || []).map(f => f.filename?.toLowerCase() || '').join(' ');

                    const match_author = !filters.author || author.includes(filters.author);
                    const match_content = !filters.content || comment_text.includes(filters.content);
                    const match_file = !filters.file || file_names.includes(filters.file);
                    const match_date = !filters.date || date_str.includes(filters.date);

                    const match_any = filters.any.length === 0 || filters.any.some(term =>
                        comment_text.includes(term) ||
                        author.includes(term) ||
                        date_str.includes(term) ||
                        file_names.includes(term)
                    );

                    return match_author && match_content && match_file && match_date && match_any;
                });

                load_comments(1);
            });

            // print them all out
            // we have comment, created_by and created_at, as well as data/x/...
            const comments_div = document.createElement('div');
            comments_div.className = 'post-comments-list';
            comments_div.id = 'post-comments-list';
            const comments_per_page = 10;
            let current_page = 1;

            function clear_comments() {
                while (comments_div.firstChild) {
                    comments_div.removeChild(comments_div.firstChild);
                }
            }

            let filtered_comments = [];
            let search_query = '';

            async function load_comments(page) {
                clear_comments();

                const comments_source = search_query ? filtered_comments : post.comments;
                const start_index = (page - 1) * comments_per_page;
                const comments_to_load = comments_source.slice(start_index, start_index + comments_per_page);

                for (const [index, comment] of comments_to_load.entries()) {
                    const comment_div = document.createElement('div');
                    comment_div.className = 'post-comment';
                    comment_div.style.textAlign = "left";

                    const comment_header = document.createElement('div');
                    comment_header.style.display = 'flex';
                    comment_header.style.alignItems = 'center';
                    comment_header.style.gap = '5px';
                    comment_header.className = 'post-comment-header';

                    const profile = await get_profile_for_user(comment.created_by);
                    if (!profile || !profile.profile_key) {
                        const icon = document.createElement('i');
                        icon.className = "fa-solid fa-circle-user";
                        icon.style.marginRight = '5px';
                        comment_header.appendChild(icon);
                    } else {
                        const profile_img = document.createElement('img');
                        profile_img.src = `/download/${profile.profile_key}`;
                        profile_img.className = 'post-comment-profile';
                        profile_img.style.marginRight = '5px';
                        profile_img.style.maxWidth = '15px';
                        profile_img.style.maxHeight = '15px';
                        profile_img.style.borderRadius = '50%';
                        comment_header.appendChild(profile_img);
                    }

                    const comment_author = document.createElement('p');
                    comment_author.style.margin = '0';
                    comment_author.style.fontWeight = 'bold';
                    comment_author.onclick = () => {
                        view_profile(comment.created_by);
                    };

                    const formatted_date = new Date(comment.created_at).toLocaleDateString();
                    comment_author.innerHTML = `${profile?.display_name || comment.created_by} on ${formatted_date}`;
                    comment_header.appendChild(comment_author);

                    const comment_content = document.createElement('div');
                    comment_content.innerHTML = comment.comment || 'No content';
                    comment_content.className = 'post-comment-content';

                    comment_div.appendChild(comment_header);
                    comment_div.appendChild(comment_content);

                    if (comment.data && comment.data.length > 0) {
                        const files_grid = document.createElement('div');
                        files_grid.className = 'post-comment-files-grid';

                        comment.data.forEach(file => {
                            const file_div = document.createElement('div');
                            file_div.className = 'post-comment-file';
                            file_div.id = file.download_key;

                            const file_name = document.createElement('p');
                            file_name.innerHTML = file.filename || 'No filename';
                            file_name.className = 'post-comment-file-name';
                            file_name.onclick = () => {
                                play_click();
                                document.location.href = `/download/${file.download_key}`;
                            }

                            file_div.appendChild(file_name);
                            files_grid.appendChild(file_div);
                        });

                        comment_div.appendChild(document.createElement('br'));
                        comment_div.appendChild(files_grid);
                    }

                    // add delete button
                    if ((post.open && post.created_by === get_cookie('username')) || get_cookie('user_type') === "1") {
                        const delete_button = document.createElement('button');
                        delete_button.innerHTML = 'Delete';
                        delete_button.className = 'post-comment-delete-button';
                        delete_button.onclick = () => {
                            play_click();
                            fetch('/api/delete_comment_post', {
                                method: 'POST',
                                headers: {
                                    'Content-Type': 'application/json'
                                },
                                body: JSON.stringify({ post_id: post_id, comment_id: index })
                            }).then(response => {
                                if (response.status === 204) {
                                    show_post(post_id, topic_id);
                                } else {
                                    console.error('Failed to delete comment');
                                }
                            });
                        };
                        comment_div.appendChild(document.createElement('br'));
                        comment_div.appendChild(delete_button);
                    }

                    comments_div.appendChild(comment_div);
                }

                current_page = page;
                render_pagination_controls();
            }

            function render_pagination_controls() {
                const old_pagination = document.getElementById('pagination_controls');
                if (old_pagination) old_pagination.remove();

                const pagination_div = document.createElement('div');
                pagination_div.id = 'pagination_controls';
                pagination_div.style.marginTop = '10px';

                const comments_source = search_query ? filtered_comments : post.comments;
                const total_pages = Math.ceil(comments_source.length / comments_per_page);

                const prev_btn = document.createElement('button');
                prev_btn.innerText = 'Prev';
                prev_btn.disabled = (current_page === 1);
                prev_btn.onclick = () => {
                    play_click();
                    load_comments(current_page - 1);
                }
                pagination_div.appendChild(prev_btn);

                for (let i = 1; i <= total_pages; i++) {
                    const page_btn = document.createElement('button');
                    page_btn.innerText = i;
                    page_btn.style.margin = '0 5px';
                    page_btn.disabled = (i === current_page);
                    page_btn.onclick = () => {
                        play_click();
                        load_comments(i);
                    }
                    pagination_div.appendChild(page_btn);
                }

                const next_btn = document.createElement('button');
                next_btn.innerText = 'Next';
                next_btn.disabled = (current_page === total_pages);
                next_btn.onclick = () => {
                    play_click();
                    load_comments(current_page + 1);
                }
                pagination_div.appendChild(next_btn);

                comments_div.after(pagination_div);
            }


            load_comments(1);

            post_window.appendChild(comments_div);
        } else {
            const no_comments = document.createElement('p');
            no_comments.innerHTML = 'No replies yet. Be the first to reply!';
            no_comments.className = 'post-no-comments';

            post_window.appendChild(no_comments);
        }

        if (post.open && (post.created_by === get_cookie('username') || get_cookie('user_type') === "1")) {
            const close_button = document.createElement('button');
            close_button.innerHTML = 'Close Post';
            close_button.className = 'post-close-button';
            close_button.onclick = () => {
                play_click();
                fetch('/api/close_post', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify({ post_id: post_id, open: false })
                }).then(response => {
                      if (response.httpRequestStatusCode !== 204) {
                          const json = JSON.parse(text);
                          if (json.error) {
                              console.error(json.error);
                          } else {
                              show_post(post_id, topic_id);
                          }
                      }
                      show_post(post_id, topic_id);
                  });
            }

            post_window.appendChild(document.createElement('br'));
            post_window.appendChild(close_button);
        } else if (!post.open && (post.created_by === get_cookie('username') || get_cookie('user_type') === "1")) {
            const reopen_button = document.createElement('button');
            reopen_button.innerHTML = 'Reopen Post';
            reopen_button.className = 'post-reopen-button';
            reopen_button.onclick = () => {
                play_click();
                fetch('/api/close_post', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify({ post_id: post_id, open: true })
                }).then(response => {
                      if (response.httpRequestStatusCode !== 204) {
                          const json = JSON.parse(text);
                          if (json.error) {
                              console.error(json.error);
                          } else {
                              show_post(post_id, topic_id);
                          }
                      }
                      show_post(post_id, topic_id);
                  });
            }

            post_window.appendChild(document.createElement('br'));
            post_window.appendChild(reopen_button);
        }

        // delete post
        if (get_cookie('user_type') === "1" || (post.open && post.created_by === get_cookie('username'))) {
            const delete_button = document.createElement('button');
            delete_button.innerHTML = 'Delete Post';
            delete_button.className = 'post-delete-button';
            delete_button.style.marginLeft = "10px";
            delete_button.onclick = () => {
                play_click();
                fetch('/api/delete_post', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify({ post_id: post_id })
                }).then(response => {
                    if (response.status === 204) {
                        show_topic(topic_id);
                    } else {
                        console.error('Failed to delete post');
                    }
                });
            }

            post_window.appendChild(delete_button);
        }
    })
}

function show_topic(topic_id = '', parent_topic_id = '') {
    hide_all_windows();
    set_path('/topic');

    if (topic_id !== '') {
        set_path('/topic/' + topic_id);
    }

    const forum = create_window('forum-window-' + (topic_id || 'root') + '-' + (parent_topic_id || 'root'), { close_button: true, classes: ["forum_window"], close_on_escape: true, function_on_close: () => {
        hide_all_windows();
        if (parent_topic_id !== '') {
            show_topic(parent_topic_id);
        } else if (topic_id !== '') {
            // dig and get the topic this topic is in
            get_topics().then(topics => {
                let parent_topic = '';
                topics.forEach(t => {
                    if (t.topics && t.topics.includes(topic_id)) {
                        parent_topic = t.identifier;
                    }
                });
                if (parent_topic !== '') {
                    show_topic(parent_topic);
                } else {
                    show_topic();
                }
            });
        } else {
            hide_all_windows();
        }
    }});

    const topics_list = document.createElement('div');
    topics_list.className = 'forum-topics-list';
    topics_list.id = 'forum-topics-list';

    let current_topic;
    const topics = get_topics();

    // iterate over topics and create elements
    topics.then(topics => {
        topics.forEach(async topic => {
            let is_ours = false;
            let current_is_subtopic = false;

            if (topic.identifier == topic_id && topic_id !== null && topic_id !== '') {
                current_topic = topic;
            }

            // check if this topic is part of any other topics' topics list
            topics.forEach(t => {
                if (t.topics && t.topics.includes(topic.identifier)) {
                    current_is_subtopic = true;
                }
                if (t.identifier === topic_id && topic_id !== '' && t.topics && t.topics.includes(topic.identifier)) {
                    is_ours = true;
                }
            });

            if (current_is_subtopic && topic_id === '') {
                return;
            }

            if (topic.identifier === topic_id && topic_id !== '') {
                return;
            }

            if (!current_is_subtopic && topic_id !== '') {
                return;
            }

            if (topic_id === '' && current_is_subtopic) {
                return;
            }

            if (!is_ours && topic_id !== '') {
                return;
            }

            const topic_div = document.createElement('div');
            topic_div.className = 'forum-topic';
            topic_div.id = topic.identifier;
            topic_div.style.padding = "10px";
            topic_div.style.textAlign = 'left';

            const title = document.createElement('strong');
            title.innerHTML = topic.title;
            title.className = 'forum-topic-title';

            const description = document.createElement('p');
            description.innerHTML = topic.description;
            description.className = 'forum-topic-description';
            if (description.innerHTML.length > 100) {
                description.innerHTML = description.innerHTML.substring(0, 100) + '...';
            }

            const topic_header = document.createElement('div');
            topic_header.style.display = 'flex';
            topic_header.style.alignItems = 'center';
            topic_header.style.gap = '5px';
            topic_header.className = 'post-topic-header';

            const profile = await get_profile_for_user(topic.created_by);
            if (!profile || !profile.profile_key) {
                const icon = document.createElement('i');
                icon.className = "fa-solid fa-circle-user";
                icon.style.marginRight = '5px';
                topic_header.appendChild(icon);
            } else {
                const profile_img = document.createElement('img');
                profile_img.src = `/download/${profile.profile_key}`;
                profile_img.className = 'post-comment-profile';
                profile_img.style.marginRight = '5px';
                profile_img.style.maxWidth = '15px';
                profile_img.style.maxHeight = '15px';
                profile_img.style.borderRadius = '50%';
                topic_header.appendChild(profile_img);
            }

            const topic_author = document.createElement('p');
            topic_author.style.margin = '0';
            topic_author.style.fontWeight = 'bold';
            topic_author.onclick = () => {
                view_profile(topic.created_by);
            };

            const formatted_date = new Date(topic.created_at).toLocaleDateString();
            topic_author.innerHTML = `${profile?.display_name || topic.created_by} on ${formatted_date}`;
            topic_header.appendChild(topic_author);

            topic_div.appendChild(title);
            topic_div.appendChild(topic_header);
            topic_div.appendChild(description);

            topic_div.onclick = () => {
                play_click();
                show_topic(topic.identifier, topic_id);
            };

            // add delete button
            if (get_cookie('user_type') === "1" || (topic.open && topic.created_by === get_cookie('username'))) {
                const delete_button = document.createElement('button');
                delete_button.innerHTML = 'Delete Topic';
                delete_button.className = 'forum-topic-delete-button';
                delete_button.onclick = () => {
                    play_click();
                    fetch('/api/delete_topic', {
                        method: 'POST',
                        headers: {
                            'Content-Type': 'application/json'
                        },
                        body: JSON.stringify({ topic_id: topic.identifier })
                    }).then(response => {
                        if (response.status === 204) {
                            show_topic(parent_topic_id);
                        } else {
                            console.error('Failed to delete topic');
                        }
                    });
                }

                topic_div.appendChild(delete_button);
            }
            // add close/reopen button
            if (get_cookie('user_type') === "1" || (topic.open && topic.created_by === get_cookie('username'))) {
                const close_button = document.createElement('button');
                close_button.innerHTML = topic.open ? 'Close Topic' : 'Reopen Topic';
                close_button.className = 'forum-topic-close-button';
                close_button.onclick = () => {
                    play_click();
                    fetch('/api/close_topic', {
                        method: 'POST',
                        headers: {
                            'Content-Type': 'application/json'
                        },
                        body: JSON.stringify({ topic_id: topic.identifier, open: !topic.open })
                    }).then(response => {
                        if (response.status === 204) {
                            show_topic(topic_id);
                        } else {
                            console.error('Failed to close/reopen topic');
                        }
                    });
                }

                topic_div.appendChild(close_button);
            }

            topics_list.appendChild(topic_div);
        });
    });

    const posts_div = document.createElement('div');
    posts_div.className = 'forum-posts-list';
    posts_div.id = 'forum-posts-list';

    const posts = get_posts(topic_id);
    posts.then(posts => {
        posts.forEach(async post => {
            if (topic_id === '') {
                return;
            }

            if (post.topic_id !== topic_id) {
                return;
            }

            const post_div = document.createElement('div');
            post_div.className = 'forum-post';
            post_div.id = post.identifier;
            post_div.style.textAlign = 'left';
            post_div.style.padding = "10px";
            post_div.onclick = () => {
                play_click();

                show_post(post.identifier, topic_id);
            }

            const post_span = document.createElement('span');
            post_span.className = 'forum-post';
            post_span.id = post.identifier + "_div";

            const title = document.createElement('p');
            title.innerHTML = post.title || 'No title';
            title.className = 'forum-post-title';

            const post_header = document.createElement('div');
            post_header.style.display = 'flex';
            post_header.style.alignItems = 'center';
            post_header.style.gap = '5px';
            post_header.className = 'post-post-header';

            const profile = await get_profile_for_user(post.created_by);
            if (!profile || !profile.profile_key) {
                const icon = document.createElement('i');
                icon.className = "fa-solid fa-circle-user";
                icon.style.marginRight = '5px';
                post_header.appendChild(icon);
            } else {
                const profile_img = document.createElement('img');
                profile_img.src = `/download/${profile.profile_key}`;
                profile_img.className = 'post-comment-profile';
                profile_img.style.marginRight = '5px';
                profile_img.style.maxWidth = '15px';
                profile_img.style.maxHeight = '15px';
                profile_img.style.borderRadius = '50%';
                post_header.appendChild(profile_img);
            }

            const post_author = document.createElement('p');
            post_author.style.margin = '0';
            post_author.style.fontWeight = 'bold';
            post_author.onclick = () => {
                view_profile(post.created_by);
            };

            const formatted_date = new Date(post.created_at).toLocaleDateString();
            post_author.innerHTML = `${profile?.display_name || post.created_by} on ${formatted_date}`;
            post_header.appendChild(post_author);

            const content = document.createElement('p');
            content.className = 'forum-post-content';

            if (post.text) {
                content.innerHTML = post.text.substring(0, 50).replace(/\n/g, ' ');
                if (post.text.length > 50) {
                    content.innerHTML += '...';
                }
            }

            post_span.appendChild(title);
            post_span.appendChild(post_header);
            post_span.appendChild(content);

            post_div.appendChild(post_span);
            posts_div.appendChild(post_div);
        });
    });

    const topics_title = document.createElement('h2');
    topics_title.innerHTML = 'Topics';
    topics_title.className = 'forum-topics-title';

    forum.appendChild(topics_title);

    if (is_logged_in() && get_cookie('user_type') === '1') {
        const create_topic_button = document.createElement('button');
        create_topic_button.innerHTML = 'Create Topic';
        create_topic_button.style.marginRight = '10px';
        create_topic_button.className = 'forum-create-topic-button';
        create_topic_button.onclick = () => {
            play_click();

            const window = create_window('create-topic-window', {classes: ["forum_window"], back_button: null, close_button: true, close_on_click_outside: true, close_on_escape: true});

            const title = document.createElement('h2');
            title.innerHTML = 'Create Topic';
            title.className = 'forum-create-topic-title';

            const paragraph = document.createElement('p');
            paragraph.innerHTML = 'Enter a title and description for your fancy topic. These will (obviously) be shown to users.';
            paragraph.className = 'forum-create-topic-paragraph';

            const title_input = document.createElement('input');
            title_input.type = 'text';
            title_input.name = 'title';
            title_input.placeholder = 'Title';

            const description_input = document.createElement('textarea');
            description_input.name = 'description';
            description_input.placeholder = 'Description';

            const closed = document.createElement('input');
            closed.type = 'checkbox';
            closed.name = 'closed';
            closed.id = 'forum-create-topic-closed';

            const closed_label = document.createElement('label');
            closed_label.for = 'closed';
            closed_label.innerHTML = 'Closed';
            closed_label.className = 'forum-create-topic-closed-label';

            const button = document.createElement('button');
            button.innerHTML = 'Create Topic';
            button.className = 'forum-create-topic-submit-button';
            button.onclick = () => {
                if (title_input.value && description_input.value) {
                    play_click();

                    const json = {
                        title: title_input.value,
                        description: description_input.value,
                    };

                    if (topic_id !== '' && topic_id !== null) {
                        json.parent_topics = [topic_id];
                    }
                    if (closed && closed.checked) {
                        json.closed = true;
                    }

                    fetch('/api/create_topic', {
                        method: 'POST',
                        headers: {
                            'Content-Type': 'application/json'
                        },
                        body: JSON.stringify(json)
                    })
                        .then(response => {
                            hide_all_windows();

                            if (response.status === 204 || response.status === 200) {
                                show_topic(parent_topic_id);
                            } else {
                                console.log("failed to create topic");
                            }
                        });
                }
            }

            window.appendChild(title);
            window.appendChild(paragraph);
            window.appendChild(title_input);
            window.appendChild(document.createElement('br'));
            window.appendChild(document.createElement('br'));
            window.appendChild(description_input);
            window.appendChild(document.createElement('br'));
            window.appendChild(document.createElement('br'));
            window.appendChild(closed);
            window.appendChild(closed_label);
            window.appendChild(document.createElement('br'));
            window.appendChild(document.createElement('br'));
            window.appendChild(button);
        };
        forum.appendChild(create_topic_button);
    }

    const posts_title = document.createElement('h2');
    posts_title.innerHTML = 'Posts';
    posts_title.className = 'forum-posts-title';

    forum.appendChild(topics_list);
    forum.appendChild(posts_title);

    if (is_logged_in() && topic_id !== '' && topic_id !== null && (get_cookie("user_type") === '1' || current_topic.open)) {
        const create_post_button = document.createElement('button');
        create_post_button.innerHTML = 'Create Post';
        create_post_button.className = 'forum-create-post-button';
        create_post_button.onclick = () => {
            play_click();

            const window = create_window('create-post-window', {classes: ["forum_window"], back_button: null, close_button: true, close_on_click_outside: true, close_on_escape: true});

            const title = document.createElement('h2');
            title.innerHTML = 'Create Post';
            title.className = 'forum-create-post-title';

            const paragraph = document.createElement('p');
            paragraph.innerHTML = 'Enter a title and content for your post. You can also upload files.';
            paragraph.className = 'forum-create-post-paragraph';

            const title_input = document.createElement('input');
            title_input.type = 'text';
            title_input.name = 'title';
            title_input.placeholder = 'Title';
            title_input.className = 'forum-create-post-title-input';

            const content_input = document.createElement('textarea');
            content_input.name = 'content';
            content_input.placeholder = 'Content';
            content_input.className = 'forum-create-post-content-input';
            content_input.style.height = '200px';
            content_input.style.width = '80%';

            const file_upload = document.createElement('input');
            file_upload.type = 'file';
            file_upload.name = 'file';
            file_upload.accept = '*/*';
            file_upload.className = 'forum-create-topic-file-upload';
            file_upload.multiple = true;

            const closed = document.createElement('input');
            closed.type = 'checkbox';
            closed.name = 'closed';
            closed.id = 'forum-create-post-closed';

            const closed_label = document.createElement('label');
            closed_label.for = 'closed';
            closed_label.innerHTML = 'Closed';
            closed_label.className = 'forum-create-post-closed-label';

            const button = document.createElement('button');
            button.innerHTML = 'Create Post';
            button.className = 'forum-create-post-submit-button';
            button.onclick = () => {
                if (content_input.value) {
                    const form_data = new FormData();

                    const json = {
                        topic_id: topic_id,
                        title: title_input.value,
                        text: content_input.value,
                    };

                    if (closed && closed.checked) {
                        json.open = !closed;
                    }

                    form_data.append('json', new Blob([JSON.stringify(json)], { type: 'application/json' }));

                    if (file_upload.files.length > 0) {
                        for (let i = 0; i < file_upload.files.length; i++) {
                            if (file_upload.files[i].name === 'json') {
                                continue;
                            }

                            form_data.append('file_' + i, file_upload.files[i]);
                        }
                    }

                    fetch('/api/create_post', {
                        method: 'POST',
                        body: form_data
                    })
                        .then(data => {
                            hide_all_windows();

                            if (data.status === 204 || data.status === 200) {
                                show_topic(topic_id);
                            } else {
                                alert('Error creating post: ' + data.error);
                            }
                        })
                        .catch(error => {
                            console.error('Error creating post:', error);
                            alert('Error creating post: ' + error.message);
                        });
                }
            }

            window.appendChild(title);
            window.appendChild(paragraph);
            window.appendChild(title_input);
            window.appendChild(document.createElement('br'));
            window.appendChild(document.createElement('br'));
            window.appendChild(content_input);
            window.appendChild(document.createElement('br'));
            window.appendChild(document.createElement('br'));
            window.appendChild(closed);
            window.appendChild(closed_label);
            window.appendChild(document.createElement('br'));
            window.appendChild(document.createElement('br'));
            window.appendChild(file_upload);
            window.appendChild(document.createElement('br'));
            window.appendChild(document.createElement('br'));
            window.appendChild(button);
        };

        forum.appendChild(create_post_button);
    }

    forum.appendChild(posts_div);
}

function show_credits() {
    set_path('/');
    hide_initial();

    const credits = create_window('credits-window', {back_button: null, close_button: false, moveable: false, close_on_click_outside: true, close_on_escape: true});
    credits.style.overflow = "hidden";
    credits.style.minWidth = "80%";
    credits.style.minHeight = "80%";
    credits.onclick = () => {
        hide_all_windows();
    }

    const roll_credits = (list, interval, restart_index = 1) => {
        let index = 0;
        let c_credit = null;

        const draw_credits = () => {
            if (index >= list.length) {
                index = restart_index;
            }

            const credit = list[index];

            const logo = document.createElement('img');
            logo.src = credit.logo;
            logo.style.marginRight = '10px';
            logo.style.zIndex = '99999';
            logo.style.width = '200px';
            logo.style.height = '200px';
            logo.style.borderRadius = '50%';
            logo.style.objectFit = 'cover';

            const name = document.createElement('h1');
            name.innerHTML = credit.name;
            name.style.color = 'white';
            name.style.zIndex = '99999';

            const role = document.createElement('h2');
            role.innerHTML = credit.role;
            role.style.color = 'white';
            role.style.zIndex = '99999';

            const container = document.createElement('div');
            container.className = 'fade-in';
            container.style.position = 'absolute';
            container.style.top = '50%';
            container.style.left = '50%';
            container.style.transform = 'translate(-50%, -50%)';
            if (credit.logo) {
                container.appendChild(logo);
            }
            if (credit.name) {
                container.appendChild(name);
            }
            if (credit.role) {
                container.appendChild(role);
            }

            if (c_credit) {
                c_credit.classList.remove('show');
                c_credit.classList.add('fade-out', 'hide');
                setTimeout(() => {
                    credits.removeChild(c_credit);
                    credits.appendChild(container);
                    requestAnimationFrame(() => {
                        container.classList.add('show');
                    });
                    c_credit = container;
                    index++;
                    setTimeout(draw_credits, interval);
                }, 1000);
            } else {
                credits.appendChild(container);
                requestAnimationFrame(() => {
                    container.classList.add('show');
                });
                c_credit = container;
                index++;
                setTimeout(draw_credits, interval, 1);
            }
        };

        draw_credits();
    };

    const list = [
        { logo: '/img/logo.svg', name: "Forwarder Factory", role: 'Forwarder Factory was made possible by...' },
        { logo: 'https://avatars.githubusercontent.com/u/166003882', name: 'Jacob Nilsson', role: 'Programming, Web Design, Maintenance' },
        { logo: 'https://avatars.githubusercontent.com/u/88589756', name: 'Gabubu', role: 'Maintenance' },
        { logo: 'https://avatars.githubusercontent.com/u/138316044', name: 'oxyzin', role: 'Graphic Design' }
    ];

    generate_stars(50, credits);
    roll_credits(list, 1500);

    const blacken = document.createElement('div');

    blacken.id = 'blacken';
    blacken.style.position = 'absolute';
    blacken.style.top = '0';
    blacken.style.left = '0';
    blacken.style.width = '100%';
    blacken.style.height = '100%';
    blacken.style.backgroundColor = 'black';
    blacken.style.opacity = '0.90';
    blacken.style.zIndex = '1';

    document.body.appendChild(blacken);
}

function setup(_error = "") {
    hide_initial();

    const win = create_window('setup-window', {back_button: null, close_button: false, close_on_escape: false, close_on_click_outside: false});

    const title = document.createElement('h1');
    title.innerHTML = 'Setup';
    title.className = 'setup-title';

    const paragraph = document.createElement('p');
    paragraph.innerHTML = 'Welcome to Forwarder Factory! Before you can use the site, you must set up an admin account.';
    paragraph.innerHTML += '<br>';

    const username_input = document.createElement('input');
    username_input.type = 'text';
    username_input.name = 'username';
    username_input.placeholder = 'Username';
    username_input.className = 'setup-input';
    username_input.id = 'setup-username';
    username_input.marginRight = '10px';

    const password_input = document.createElement('input');
    password_input.type = 'password';
    password_input.name = 'password';
    password_input.placeholder = 'Password';
    password_input.className = 'setup-input';
    password_input.id = 'setup-password';
    password_input.marginRight = '10px';

    const email = document.createElement('input');
    email.type = 'email';
    email.name = 'email';
    email.placeholder = 'Email';
    email.className = 'setup-input';
    email.id = 'setup-email';
    email.marginRight = '10px';

    const submit = document.createElement('button');
    submit.innerHTML = 'Submit';
    submit.className = 'setup-button';
    submit.id = 'setup-submit';
    submit.onclick = () => {
        const json = {
            username: username_input.value,
            password: password_input.value,
            email: email.value
        };

        fetch('/try_setup', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(json),
        })
            .then(response => {
                if (response.status === 204) {
                    window.location.href = '/';
                }

                return response.json();
            })
            .then(json => {
                if (json.error_str) {
                   setup(json.error_str);
                }

                throw new Error('Failed to set up admin account');
            })
            .catch((error) => {
                console.error('Error:', error);
            });
    }

    win.appendChild(title);
    win.appendChild(paragraph);
    win.appendChild(username_input);
    win.appendChild(password_input);
    win.appendChild(email);
    win.appendChild(document.createElement('br'));

    if (_error) {
        const error = document.createElement('p');
        error.className = 'error';
        error.innerHTML = _error;
        win.appendChild(error);
    }

    win.appendChild(document.createElement('br'));
    win.appendChild(submit);
}
function get_grid(elements) {
    const container = document.createElement('div');
    container.className = 'grid';

    container.style.display = 'flex';
    container.style.justifyContent = 'center';
    container.style.alignItems = 'center';
    container.style.height = '100vh';
    container.style.width = '100vw';

    const grid = document.createElement('div');
    grid.className = 'grid-inner';

    grid.style.display = 'flex';
    grid.style.flexWrap = 'wrap';
    grid.style.gap = '10px';
    grid.style.justifyContent = 'center';
    grid.style.maxWidth = '90vw';

    elements.forEach(element => {
        element.classList.add('grid-item');
        grid.appendChild(element);
    });

    container.appendChild(grid);
    return container;
}



function get_link_box(p) {
    const link_box = document.createElement('div');
    link_box.className = 'link_box';
    link_box.style.overflow = 'hidden';
    link_box.style.position = 'relative';
    link_box.style.zIndex = '1';

    if (p.id) {
        link_box.id = p.id;
    }

    if (p.location) {
        link_box.onclick = () => location.href = p.location;
        link_box.style.cursor = 'pointer';
    } else if (p.onclick) {
        link_box.setAttribute('onclick', p.onclick);
    }

    if (p.background_color) {
        link_box.style.backgroundColor = p.background_color;
    }
    if (p.color) {
        link_box.style.color = p.color;
    }

    if (p.img) {
        const icon = document.createElement('img');
        icon.src = p.img;
        icon.alt = '';
        icon.style.width = '24px';
        icon.style.verticalAlign = 'middle';
        icon.style.marginRight = '8px';
        link_box.appendChild(icon);
    }

    const title = document.createElement('h2');
    title.className = 'link_box_title';
    title.textContent = p.title;
    link_box.appendChild(title);

    const description = document.createElement('p');
    description.className = 'link_box_description';
    description.textContent = p.description;
    link_box.appendChild(description);

    return link_box;
}

function init_page() {
    let list = [];
    list.push(get_link_box({
        title: "Browse",
        description: "Browse channels uploaded by others.",
        background_color: "",
        id: "browse-button",
        onclick: "hide_all_windows(); play_click(); show_browse()"
    }));
    list.push(get_link_box({
        title: "Sandbox",
        description: "Check out files uploaded by users.",
        background_color: "",
        id: "sandbox-button",
        onclick: "hide_all_windows(); play_click(); show_sandbox()"
    }))
    list.push(get_link_box({
       title: "Forum",
       description: "Check out the Forwarder Factory forum.",
        background_color: "",
        id: "forum-button",
        onclick: "hide_all_windows(); play_click(); show_topic()",
    }))

    if (get_cookie("username") === null) {
        list.push(get_link_box({
            title: "Log in",
            description: "Log in to your account.",
            id: "login-button",
            onclick: "hide_all_windows(); play_click(); show_login()"
        }));
        list.push(get_link_box({
            title: "Register",
            description: "Register a new account.",
            id: "register-button",
            onclick: "hide_all_windows(); play_click(); show_register()"
        }));
    } else {
        if (get_cookie("user_type") === "1") {
            list.push(get_link_box({
                title: "Admin",
                description: "Access the admin panel.",
                id: "admin-button",
                onclick: "hide_all_windows(); play_click(); show_admin()"
            }));
        }
        list.push(get_link_box({
            title: "Upload",
            description: "Upload a forwarder or channel.",
            id: "upload-button",
            onclick: "hide_all_windows(); play_click(); show_upload()"
        }));
        list.push(get_link_box({
            title: "Log out",
            description: "Log out of your account.",
            id: "logout-button",
            onclick: "hide_all_windows(); play_click(); show_logout()"
        }));
    }

    list.push(get_link_box({
       title: "Discord",
       description: "Join our awesome Discord server.",
       id: "discord-button",
       onclick: "hide_all_windows(); play_click(); show_discord()",
    }));
    list.push(get_link_box({
        title: "Announcements",
        description: "View the latest announcements.",
        id: "announcements-button",
        onclick: "hide_all_windows(); play_click(); get_announcements()",
    }));
    list.push(get_link_box({
        title: "Credits",
        description: "View the credits for Forwarder Factory.",
        id: "credits-button",
        onclick: "hide_all_windows(); play_click(); show_credits()"
    }));

    const grid = get_grid(list, 'initial-link-grid');
    document.body.appendChild(grid);

    // special case: credits-button should have fancy stars
    // just copy from the credits window
    const credits_button = document.getElementById('credits-button');
    if (credits_button) {
        generate_stars(50, credits_button);
    }

    // special case: discord-button should have a grid of sprites
    const discord_button = document.getElementById('discord-button');
    if (discord_button) {
        generate_sprites(discord_button, '/img/discord.svg');
    }

    const announcements_button = document.getElementById('announcements-button');
    if (announcements_button) {
        generate_sprites(announcements_button, '/img/announcements.svg', { invert: true, opacity: 0.5 });
    }

    const browse_button = document.getElementById('browse-button');
    if (browse_button) {
        const deg = 1000;
        generate_sprites(browse_button, '/img/background-logo-1.png', { opacity: 0.2, hue: deg });
        // on hover, random colors
        browse_button.onmouseover = () => {
            const sprites = browse_button.querySelectorAll('.sprite');
            sprites.forEach(sprite => {
                sprite.style.filter = `hue-rotate(${Math.random() * 360}deg)`;
            });
        };
        browse_button.onmouseleave = () => {
            // revert
            const sprites = browse_button.querySelectorAll('.sprite');
            sprites.forEach(sprite => {
                sprite.style.filter = 'hue-rotate(' + deg + 'deg)';
            });
        }
    }

    const sandbox_button = document.getElementById('sandbox-button');
    if (sandbox_button) {
        generate_sprites(sandbox_button, '/img/shovel.svg', { opacity: 0.1 });
    }

    const forum_button = document.getElementById('forum-button');
    if (forum_button) {
        generate_sprites(forum_button, '/img/messages.svg', { opacity: 0.1 });
    }

    const login_button = document.getElementById('login-button');
    if (login_button) {
        generate_sprites(login_button, '/img/question-mark-block.svg', { opacity: 0.1 });
    }

    const register_button = document.getElementById('register-button');
    if (register_button) {
        generate_sprites(register_button, '/img/coin.svg', { opacity: 0.1 });
    }

    const admin_button = document.getElementById('admin-button');
    if (admin_button) {
        generate_sprites(admin_button, '/img/hammer.svg', { opacity: 0.1 });
    }

    const upload_button = document.getElementById('upload-button');
    if (upload_button) {
        generate_sprites(upload_button, '/img/retro-star.svg', { opacity: 0.1 });
    }

    const logout_button = document.getElementById('logout-button');
    if (logout_button) {
        generate_sprites(logout_button, '/img/wave.svg', { opacity: 0.1 });
    }
}

async function get_profile_for_user(username) {
    const api = '/api/get_profile';
    const body = JSON.stringify({ usernames: [username] });

    try {
        const response = await fetch(api, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: body
        });

        if (!response.ok) {
            throw new Error(`Server error: ${response.status}`);
        }

        const data = await response.json();

        // make sure the structure is what you expect
        if (data.users && data.users[username]) {
            return data.users[username];
        } else {
            throw new Error(`User ${username} not found in response`);
        }
    } catch (error) {
        console.error('Failed to fetch profile:', error);
        return null;
    }
}

document.addEventListener('DOMContentLoaded', async () => {
    // todo: find replacement that doesn't utilize kit nonsense.
    // i hate being dependent on a third party for something as trivial as icons
    include('https://kit.fontawesome.com/aa55cd1c33.js');

    if (!is_phone()) {
        WSCBackgroundRepeatingSpawner();
    }

    let username = get_cookie('username');
    let display_name = username;
    let profile_key;

    const profile = await get_profile_for_user(username);
    if (profile) {
        display_name = profile.display_name || username;
        profile_key = profile.profile_key;
    }

    init_page();

    // required since we're not redirecting on address bar override
    window.addEventListener('popstate', () => {
        location.reload();
    });

    if (get_path() === "/login" && !is_logged_in()) show_login();
    if (get_path() === "/register" && !is_logged_in()) show_register();
    if (get_path() === "/upload" && is_logged_in()) show_upload();
    if (get_path() === "/upload" && !is_logged_in()) show_login();
    if (get_path() === "/browse") show_browse();
    if (get_path() === "/sandbox") show_sandbox();
    if (get_path() === "/admin" && is_logged_in()) show_admin();
    if (get_path() === "/admin" && !is_logged_in()) show_login();
    if (get_path() === "/logout" && is_logged_in()) show_logout();
    if (get_path() === "/topic") show_topic();

    if (get_path().startsWith("/view/")) {
        const id = get_path().substring(6);
        show_forwarder(id);
    }
    if (get_path().startsWith("/file/")) {
        const id = get_path().substring(6);
        show_file(id);
    }
    if (get_path().startsWith("/profile/")) {
        const name = get_path().substring(9);
        view_profile(name);
    }
    if (get_path().startsWith("/topic/")) {
        const topic_id = get_path().substring(7);
        show_topic(topic_id);
    }
    if (get_path().startsWith("/post/")) {
        const post_id = get_path().substring(6);
        show_post(post_id);
    }

    print_username(username, display_name, profile_key);
});