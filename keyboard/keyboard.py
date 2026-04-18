import argparse
import os
import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, Gdk

# Filas del teclado (sin números)
ROWS = [
    ['q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '´', '`'],
    ['a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'ñ', 'ç'],
    ['⇧', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-', '⇧'],
]

# Combinaciones de teclas muertas
DEAD_ACUTE = {
    'a': 'á', 'e': 'é', 'i': 'í', 'o': 'ó', 'u': 'ú',
    'A': 'Á', 'E': 'É', 'I': 'Í', 'O': 'Ó', 'U': 'Ú',
}
DEAD_GRAVE = {
    'a': 'à', 'e': 'è', 'i': 'ì', 'o': 'ò', 'u': 'ù',
    'A': 'À', 'E': 'È', 'I': 'Ì', 'O': 'Ò', 'U': 'Ù',
}
DEAD_KEYS = {'´': DEAD_ACUTE, '`': DEAD_GRAVE}

LETTERS = set('qwertyuiopasdfghjklzxcvbnmñç')
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))


class KeyboardWindow(Gtk.Window):
    def __init__(self, title='', initial_text=''):
        super().__init__(title='Teclado')
        self.shift_state = 0   # 0=off, 1=single, 2=caps lock
        self.dead_key = None   # '´' o '`' si hay tecla muerta pendiente
        self.shift_buttons = []
        self.dead_buttons = {}   # char → button
        self.letter_buttons = []  # (button, lowercase_char)

        self.fullscreen()
        self.set_decorated(False)
        self._apply_css()

        vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=6)
        vbox.set_margin_top(12)
        vbox.set_margin_bottom(12)
        vbox.set_margin_start(12)
        vbox.set_margin_end(12)
        self.add(vbox)

        if title:
            lbl = Gtk.Label()
            lbl.set_markup(
                f'<span font="26" weight="bold">{glib_escape(title)}</span>'
            )
            lbl.set_halign(Gtk.Align.START)
            vbox.pack_start(lbl, False, False, 0)

        self.entry = Gtk.Entry()
        self.entry.set_text(initial_text)
        self.entry.set_position(-1)
        self.entry.connect('activate', self.on_enter)
        vbox.pack_start(self.entry, False, False, 4)

        for row in ROWS:
            hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=4)
            for key in row:
                btn = self._make_key(key)
                hbox.pack_start(btn, True, True, 0)
            vbox.pack_start(hbox, True, True, 0)

        # Fila inferior
        bottom = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=4)

        clear = Gtk.Button(label='Borrar todo')
        clear.get_style_context().add_class('key')
        clear.get_style_context().add_class('key-clear')
        clear.connect('clicked', self.on_clear)

        space = Gtk.Button(label='Espacio')
        space.get_style_context().add_class('key')
        space.get_style_context().add_class('key-space')
        space.connect('clicked', self.on_key, ' ')

        backspace = Gtk.Button(label='⌫')
        backspace.get_style_context().add_class('key')
        backspace.get_style_context().add_class('key-backspace')
        backspace.connect('clicked', self.on_key, '⌫')

        enter = Gtk.Button(label='Intro ↵')
        enter.get_style_context().add_class('key')
        enter.get_style_context().add_class('key-enter')
        enter.connect('clicked', self.on_enter)

        bottom.pack_start(clear, True, True, 0)
        bottom.pack_start(space, True, True, 0)
        bottom.pack_start(backspace, True, True, 0)
        bottom.pack_start(enter, True, True, 0)
        vbox.pack_start(bottom, True, True, 0)

    def _apply_css(self):
        css_path = os.path.join(SCRIPT_DIR, 'keyboard.css')
        provider = Gtk.CssProvider()
        provider.load_from_path(css_path)
        Gtk.StyleContext.add_provider_for_screen(
            Gdk.Screen.get_default(),
            provider,
            Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION
        )

    def _make_key(self, key):
        btn = Gtk.Button(label=key)
        btn.get_style_context().add_class('key')
        btn.connect('clicked', self.on_key, key)

        if key == '⇧':
            btn.get_style_context().add_class('key-shift')
            self.shift_buttons.append(btn)
        elif key in DEAD_KEYS:
            btn.get_style_context().add_class('key-dead')
            self.dead_buttons[key] = btn
        elif key in LETTERS:
            self.letter_buttons.append((btn, key))

        return btn

    def on_key(self, widget, key):
        if key == '⌫':
            pos = self.entry.get_position()
            text = self.entry.get_text()
            if pos > 0:
                self.entry.set_text(text[:pos - 1] + text[pos:])
                self.entry.set_position(pos - 1)
            return

        if key == '⇧':
            self.shift_state = (self.shift_state + 1) % 3
            self._update_shift()
            return

        if key in DEAD_KEYS:
            if self.dead_key == key:
                # Segunda pulsación: desactiva
                self.dead_key = None
            else:
                self.dead_key = key
            self._update_dead_keys()
            return

        # Carácter normal
        char = key.upper() if self.shift_state > 0 and key in LETTERS else key
        if self.dead_key is not None:
            char = DEAD_KEYS[self.dead_key].get(char, self.dead_key + char)
            self.dead_key = None
            self._update_dead_keys()

        pos = self.entry.get_position()
        text = self.entry.get_text()
        self.entry.set_text(text[:pos] + char + text[pos:])
        self.entry.set_position(pos + 1)

        if self.shift_state == 1:
            self.shift_state = 0
            self._update_shift()

    def on_clear(self, widget):
        self.entry.set_text('')
        self.entry.set_position(0)

    def _update_shift(self):
        for btn in self.shift_buttons:
            ctx = btn.get_style_context()
            ctx.remove_class('shift-on')
            ctx.remove_class('shift-lock')
            if self.shift_state == 1:
                ctx.add_class('shift-on')
            elif self.shift_state == 2:
                ctx.add_class('shift-lock')

        for btn, lower_char in self.letter_buttons:
            btn.set_label(lower_char.upper() if self.shift_state > 0 else lower_char)

    def _update_dead_keys(self):
        for char, btn in self.dead_buttons.items():
            ctx = btn.get_style_context()
            if self.dead_key == char:
                ctx.add_class('dead-active')
            else:
                ctx.remove_class('dead-active')

    def on_enter(self, widget):
        print(self.entry.get_text())
        Gtk.main_quit()


def glib_escape(text):
    return (text.replace('&', '&amp;')
                .replace('<', '&lt;')
                .replace('>', '&gt;'))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--title', default='')
    parser.add_argument('--text', default='')
    args = parser.parse_args()

    win = KeyboardWindow(title=args.title, initial_text=args.text)
    win.connect('destroy', Gtk.main_quit)
    win.show_all()
    Gtk.main()
