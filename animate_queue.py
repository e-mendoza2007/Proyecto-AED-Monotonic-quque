import json
from manim import *


BOX_SIZE = 0.9
BOX_GAP = 0.15
ARR_Y = 2.2
QUEUE_Y = -1.0

COLOR_DEFAULT = "#3B82F6"
COLOR_IN_QUEUE = "#F59E0B"
COLOR_POPPED = "#EF4444"
COLOR_MAX = "#22C55E"
COLOR_WINDOW = "#94A3B8"


def load_steps(path):
    with open(path, "r") as f:
        return json.load(f)


class MonotonicQueueScene(Scene):

    CASO_JSON = "steps_general.json"

    def construct(self):
        data = load_steps(self.CASO_JSON)
        arr = data["arreglo"]
        k = data["k"]
        pasos = data["pasos"]

        titulo = Text(f"Monotonic Queue: Sliding Window Maximum (k={k})", font_size=32)
        titulo.to_edge(UP)
        self.play(Write(titulo))


        intro = VGroup(
            Text("TDA: Cola con propiedad de orden (FIFO + monotonia).", font_size=26),
            Text("Mantiene sus elementos en orden decreciente por valor.", font_size=26),
            Text("Permite hallar el maximo de cada ventana en O(1).", font_size=26),
        ).arrange(DOWN, aligned_edge=LEFT)
        intro.next_to(titulo, DOWN, buff=0.5)
        self.play(FadeIn(intro))
        self.wait(1.5)
        self.play(FadeOut(intro))

        arr_boxes = VGroup()
        arr_labels = VGroup()
        n = len(arr)
        start_x = -((n - 1) * (BOX_SIZE + BOX_GAP)) / 2

        for i, v in enumerate(arr):
            box = Square(side_length=BOX_SIZE, color=COLOR_DEFAULT)
            box.move_to([start_x + i * (BOX_SIZE + BOX_GAP), ARR_Y, 0])
            label = Text(str(v), font_size=28).move_to(box.get_center())
            idx_label = Text(str(i), font_size=18, color=GRAY).next_to(box, DOWN, buff=0.08)
            arr_boxes.add(box)
            arr_labels.add(VGroup(label, idx_label))

        self.play(
            *[Create(b) for b in arr_boxes],
            *[FadeIn(l) for l in arr_labels],
        )
        self.wait(0.5)


        queue_label_title = Text("Cola (frente = maximo actual)", font_size=24)
        queue_label_title.move_to([0, QUEUE_Y + 1.0, 0])
        self.play(FadeIn(queue_label_title))

        window_rect = None
        queue_box_group = VGroup()
        result_text = None
        max_history = VGroup()

        def box_center_in_array(i):
            return [start_x + i * (BOX_SIZE + BOX_GAP), ARR_Y, 0]

        for step in pasos:
            op = step["op"]

            if op == "push_back":
                i = step["index"]
                v = step["value"]
                # Resaltar el elemento en el arreglo
                self.play(arr_boxes[i].animate.set_color(COLOR_IN_QUEUE), run_time=0.3)
                # Crear una copia que "cae" hacia la cola
                clone = arr_boxes[i].copy().set_color(COLOR_IN_QUEUE)
                target_x = -3 + len(queue_box_group) * (BOX_SIZE + BOX_GAP)
                self.play(clone.animate.move_to([target_x, QUEUE_Y, 0]), run_time=0.4)
                queue_box_group.add(clone)

            elif op == "pop_back":
                if len(queue_box_group) > 0:
                    last = queue_box_group[-1]
                    self.play(last.animate.set_color(COLOR_POPPED), run_time=0.2)
                    self.play(FadeOut(last), run_time=0.3)
                    queue_box_group.remove(last)
                    self._reflow_queue(queue_box_group)

            elif op == "pop_front":
                if len(queue_box_group) > 0:
                    first = queue_box_group[0]
                    self.play(first.animate.set_color(COLOR_POPPED), run_time=0.2)
                    self.play(FadeOut(first), run_time=0.3)
                    queue_box_group.remove(first)
                    self._reflow_queue(queue_box_group)

            elif op == "record_max":
                m = step["max_valor"]
                ws, we = step["ventana_inicio"], step["ventana_fin"]

                # Marco de la ventana deslizante sobre el arreglo
                left = box_center_in_array(ws)[0] - BOX_SIZE / 2 - 0.05
                right = box_center_in_array(we)[0] + BOX_SIZE / 2 + 0.05
                new_rect = Rectangle(
                    width=right - left, height=BOX_SIZE + 0.2,
                    color=COLOR_WINDOW,
                ).move_to([(left + right) / 2, ARR_Y, 0])

                if window_rect is None:
                    window_rect = new_rect
                    self.play(Create(window_rect), run_time=0.3)
                else:
                    self.play(Transform(window_rect, new_rect), run_time=0.3)

                if len(queue_box_group) > 0:
                    self.play(queue_box_group[0].animate.set_color(COLOR_MAX), run_time=0.2)

                max_tag = Text(f"max = {m}", font_size=22, color=COLOR_MAX)
                max_tag.next_to(window_rect, DOWN, buff=1.4)
                if result_text is None:
                    result_text = max_tag
                    self.play(FadeIn(result_text))
                else:
                    self.play(Transform(result_text, max_tag), run_time=0.3)

                if len(queue_box_group) > 0:
                    self.play(queue_box_group[0].animate.set_color(COLOR_IN_QUEUE), run_time=0.2)

        self.wait(1)


        self.play(*[FadeOut(m) for m in self.mobjects])
        complejidad = VGroup(
            Text("Analisis de complejidad", font_size=30, weight=BOLD),
            Text("push / pop_front / pop_back: O(1) amortizado por elemento", font_size=26),
            Text("Recorrido de n elementos: O(n) en total (cada indice entra", font_size=26),
            Text("y sale de la cola a lo mas una vez)", font_size=26),
            Text("Memoria: O(k) en el peor caso (tamano de la ventana)", font_size=26),
        ).arrange(DOWN, aligned_edge=LEFT)
        self.play(Write(complejidad))
        self.wait(2)

    def _reflow_queue(self, queue_box_group):
        anims = []
        for pos, box in enumerate(queue_box_group):
            target_x = -3 + pos * (BOX_SIZE + BOX_GAP)
            anims.append(box.animate.move_to([target_x, QUEUE_Y, 0]))
        if anims:
            self.play(*anims, run_time=0.3)


class CasoBordeScene(MonotonicQueueScene):

    CASO_JSON = "steps_un_elemento.json"