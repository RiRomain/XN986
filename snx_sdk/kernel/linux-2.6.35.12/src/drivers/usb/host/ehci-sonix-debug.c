


#define snx_ehci_dbg(ehci, fmt, args...) \
	dev_dbg (ehci_to_hcd(ehci)->self.controller , fmt , ## args )

static void __maybe_unused
snx_dbg_qtd (const char *label, struct ehci_hcd *ehci, struct ehci_qtd *qtd)
{
	printk ("%s (%p):\n\tn=%08x %08x t=%08x p0=%08x", label, qtd,
		hc32_to_cpup(ehci, &qtd->hw_next),
		hc32_to_cpup(ehci, &qtd->hw_alt_next),
		hc32_to_cpup(ehci, &qtd->hw_token),
		hc32_to_cpup(ehci, &qtd->hw_buf [0]));

	printk (" p1=%08x p2=%08x p3=%08x p4=%08x\n",
		hc32_to_cpup(ehci, &qtd->hw_buf[1]),
		hc32_to_cpup(ehci, &qtd->hw_buf[2]),
		hc32_to_cpup(ehci, &qtd->hw_buf[3]),
		hc32_to_cpup(ehci, &qtd->hw_buf[4]));
}

static void __maybe_unused
snx_dbg_qh (const char *label, struct ehci_hcd *ehci, struct ehci_qh_hw *qh)
{
	printk ("%s (%p):\n\tn=%08x info=%x %x cur=%x\n", label,
		qh, qh->hw_next, qh->hw_info1, qh->hw_info2, qh->hw_current);

	snx_dbg_qtd("overlay", ehci, (struct ehci_qtd *) &qh->hw_qtd_next);
}

static void snx_dbg_asyc_list (struct ehci_hcd *ehci)
{
	int h, t;		/* for counter */
	unsigned int head;
	struct ehci_qh_hw *qh;
	struct ehci_qtd *qtd;
	char str[16];

	printk ("\n!!!! snx_dbg_asyc_list (0x%x):\n", ehci->regs->async_next);
	head = ehci->regs->async_next & ~0x1f;

	/* Trace qH */
	qh = (struct ehci_qh_hw *) phys_to_virt (head);
	for (h = 0; ; ++h) {
		/* Print qH */
		sprintf (str, "qH(%d)", h);
		snx_dbg_qh (str, ehci, qh);

		/* Print Current qTD */
		qtd = (struct ehci_qtd *) phys_to_virt (qh->hw_current & ~0x1f);
		sprintf (str, "qTD(0)", t);
		snx_dbg_qtd (str, ehci, qtd);

		if (!(qh->hw_qtd_next & EHCI_LIST_END(ehci))) {
			/* Trace Next qTD */
			qtd = (struct ehci_qtd *)
				phys_to_virt (qh->hw_qtd_next & ~0x1f);
			for (t = 1; ; ++t) {
				/* Print qTD */
				sprintf (str, "qTD(%d)", t);
				snx_dbg_qtd (str, ehci, qtd);

				if (qtd->hw_next & EHCI_LIST_END(ehci))
					break;
				qtd = (struct ehci_qtd *)
					phys_to_virt (qtd->hw_next & ~0x1f);
			}
		}

		if ((qh->hw_next & EHCI_LIST_END(ehci)) ||
				((qh->hw_next & ~0x1f) == head))
			break;
		qh = (struct ehci_qh_hw *) phys_to_virt (qh->hw_next & ~0x1f);
	}
}

static void snx_dbg_reg_dump (struct ehci_hcd *ehci)
{
	int i;
	unsigned int *addr = (unsigned int *)ehci->caps;

	printk ("\n!!!! snx_dbg_reg_dump (%p):\n", addr);

	for (i = 0 ; i < 52; i += 4)
		printk ("%02x: %08x %08x %08x %08x\n", (i * 4),
			*(addr + i), *(addr + i + 1),
			*(addr + i + 2), *(addr + i + 3));
	printk ("snx_dbg_reg_dump end.\n");
}

static void snx_dbg_fatal (struct ehci_hcd *ehci)
{
	snx_dbg_reg_dump (ehci);
	if (ehci->regs->command & CMD_ASE)
		snx_dbg_asyc_list (ehci);
}

